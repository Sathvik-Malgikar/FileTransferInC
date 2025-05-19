#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define SERVER_PORT 65432
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define CHUNK_SIZE 512

// Structure to hold client connection details
typedef struct {
    int socket;
    struct sockaddr_in address;
    int id;
} client_t;

// Global variables
client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t client_sem;

// Function to calculate SHA-256 checksum (simplified for demonstration)
// Note: This is a placeholder.  For real applications, use a proper library
// like OpenSSL.  This simplified version is NOT cryptographically secure.
char *calculate_checksum(const char *data, size_t data_len) {
    if (data == NULL || data_len == 0) {
        return strdup("INVALID_DATA"); // Return a constant error string
    }
    unsigned long hash = 5381;
    for (size_t i = 0; i < data_len; i++) {
        hash = ((hash << 5) + hash) + data[i]; /* hash * 33 + c */
    }
    char *hash_str = malloc(32);  // Enough for a long
    if (hash_str == NULL) {
         perror("Memory allocation error in calculate_checksum");
         return NULL;
    }
    snprintf(hash_str, 32, "%lu", hash);
    return hash_str;
}

// Function to send data with error simulation
int send_data(int socket, const char *data, size_t data_len, float error_rate) {
    size_t total_sent = 0;
    while (total_sent < data_len) {
        size_t remaining = data_len - total_sent;
        size_t send_size = remaining < MAX_BUFFER_SIZE ? remaining : MAX_BUFFER_SIZE;

        // Simulate error
        if (((float)rand() / RAND_MAX) < error_rate) {
            if (((float)rand() / RAND_MAX) < 0.5) {
                // Simulate packet loss
                printf("Simulating packet loss\n");
                usleep(100000); // Simulate a small delay
                continue;
            } else {
                // Simulate data corruption (very simple corruption)
                char *corrupted_data = malloc(send_size);
                if (corrupted_data == NULL){
                    perror("Memory allocation error in send_data");
                    return -1;
                }
                memcpy(corrupted_data, data + total_sent, send_size);
                corrupted_data[0] = corrupted_data[0] ^ 0x0F; // Flip some bits
                printf("Simulating data corruption\n");
                if (send(socket, corrupted_data, send_size, 0) == -1) {
                    perror("Error sending corrupted data");
                    free(corrupted_data);
                    return -1;
                }
                free(corrupted_data);
                total_sent += send_size;
                usleep(100000);
                continue;

            }
        }
        ssize_t sent = send(socket, data + total_sent, send_size, 0);
        if (sent == -1) {
            perror("Error sending data");
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}

// Function to receive data
char *receive_data(int socket, size_t *data_len) {
    *data_len = 0;
    char *buffer = NULL;
    size_t total_received = 0;

    while (1) {
        char temp_buffer[MAX_BUFFER_SIZE];
        ssize_t received = recv(socket, temp_buffer, MAX_BUFFER_SIZE, 0);
        if (received == -1) {
            perror("Error receiving data");
            free(buffer);
            return NULL;
        }
        if (received == 0) {
            break; // Connection closed
        }
        if (total_received + received > *data_len) {
             buffer = realloc(buffer, total_received + received);
             if (buffer == NULL){
                perror("Memory allocation error in receive_data");
                return NULL;
             }
            *data_len = total_received + received;
        }
       
        memcpy(buffer + total_received, temp_buffer, received);
        total_received += received;
        if (received < MAX_BUFFER_SIZE){
            break;
        }
    }
    *data_len = total_received;
    return buffer;
}

// Function to handle a client connection
void *handle_client_connection(void *arg) {
    client_t *client = (client_t *)arg;
    int client_socket = client->socket;
    struct sockaddr_in client_address = client->address;
    int client_id = client->id;

    printf("Thread %d: New connection from %s:%d\n", client_id, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    size_t file_size;
    char *file_data = NULL;
    char *received_checksum = NULL;

    // Receive file size
    if (recv(client_socket, &file_size, sizeof(size_t), 0) == -1) {
        perror("Error receiving file size");
        goto cleanup;
    }
     printf("Thread %d: Receiving file of size %zu bytes\n", client_id, file_size);

    // Send acknowledgment
    if (send(client_socket, "OK", 2, 0) == -1) {
        perror("Error sending acknowledgment");
        goto cleanup;
    }

    // Receive file data
    file_data = receive_data(client_socket, &file_size);
    if (file_data == NULL) {
        goto cleanup;
    }
    printf("Thread %d: Received %zu bytes of file data\n", client_id, file_size);

    // Calculate checksum
    char *checksum = calculate_checksum(file_data, file_size);
     if (checksum == NULL) {
        goto cleanup;
    }
    printf("Thread %d: Calculated checksum: %s\n", client_id, checksum);

    // Send checksum
    if (send_data(client_socket, checksum, strlen(checksum), 0) == -1) {
        perror("Error sending checksum");
        free(checksum);
        goto cleanup;
    }
     free(checksum);

    // Split file into chunks and send them back
     size_t num_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE; //avoid zero size chunks
     if (send_data(client_socket, &num_chunks, sizeof(size_t), 0) == -1) {
        perror("Error sending number of chunks");
        goto cleanup;
    }
    printf("Thread %d: Number of chunks: %zu\n", client_id, num_chunks);
    if (send(client_socket, "OK", 2, 0) == -1) {
        perror("Error sending ack");
        goto cleanup;
    }

    for (size_t i = 0; i < num_chunks; i++) {
        size_t chunk_start = i * CHUNK_SIZE;
        size_t chunk_end = (i + 1) * CHUNK_SIZE > file_size ? file_size : (i + 1) * CHUNK_SIZE;
        size_t chunk_size = chunk_end - chunk_start;

        char *chunk_data = malloc(chunk_size);
        if (chunk_data == NULL) {
            perror("Memory allocation error");
            goto cleanup;
        }
        memcpy(chunk_data, file_data + chunk_start, chunk_size);

        // Send chunk data with error simulation
        if (send_data(client_socket, chunk_data, chunk_size, 0.2) == -1) { // 20% error rate
            perror("Error sending chunk data");
            free(chunk_data);
            goto cleanup;
        }
        free(chunk_data);
        char ack[MAX_BUFFER_SIZE];
         if (recv(client_socket, ack, MAX_BUFFER_SIZE, 0) == -1) {
            perror("Error receiving ack");
            goto cleanup;
        }
        if (strcmp(ack, "RETRANSMIT") == 0){
             i--;
             continue;
        }
    }
    if (send(client_socket, "TRANSFER_COMPLETE", 15, 0) == -1) {
            perror("Error sending transfer complete");
            goto cleanup;
    }
    if (recv(client_socket, ack, MAX_BUFFER_SIZE, 0) == -1) {
            perror("Error receiving final ack");
            goto cleanup;
    }
    printf("Thread %d: Transfer result: %s\n", client_id, ack);

cleanup:
    free(file_data);
    close(client_socket);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].id == client_id) {
            // Remove the client from the array
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
    sem_post(&client_sem); //signal that a client slot is free
    printf("Thread %d: Connection closed\n", client_id);
    return NULL;
}

int main() {
    int server_socket;
    struct sockaddr_in server_address;

    // Seed the random number generator
    srand(time(NULL));

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Bind socket
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening for connections");
        close(server_socket);
        return 1;
    }
     sem_init(&client_sem, 0, MAX_CLIENTS); // Initialize the semaphore

    printf("Server listening on port %d\n", SERVER_PORT);

    // Accept connections in a loop
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue; // Go back to accepting connections
        }

        sem_wait(&client_sem); // Wait for a free client slot.
        // Add client to the array and increment the count
        pthread_mutex_lock(&client_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count].socket = client_socket;
            clients[client_count].address = client_address;
            clients[client_count].id = client_count + 1;
            client_count++;
        } else {
            //reject the client
            char msg[] = "Server is busy";
            send(client_socket, msg, strlen(msg), 0);
            close(client_socket);
            pthread_mutex_unlock(&client_mutex);
            sem_post(&client_sem);
            continue;
        }
        pthread_mutex_unlock(&client_mutex);

        // Create a thread to handle the client connection
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client_connection, (void *)&clients[client_count-1]) != 0) {
            perror("Error creating thread");
            close(client_socket);
            pthread_mutex_lock(&client_mutex);
            client_count--;
            pthread_mutex_unlock(&client_mutex);
            sem_post(&client_sem);
            continue;
        }
        pthread_detach(thread); // Detach the thread
    }

    // This point will not be reached, but included for completeness
    close(server_socket);
    return 0;
}


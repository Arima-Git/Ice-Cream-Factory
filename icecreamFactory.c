#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/syscall.h>
#include <linux/kernel.h>

#define FLAVOR_1_PRICE 1.05
#define FLAVOR_2_PRICE 2.00
#define FLAVOR_3_PRICE 1.67
#define TOPPING_1_PRICE 0.8
#define TOPPING_2_PRICE 0.5

int total_tickets = 30, flavors[3] = {29, 34, 18}, toppings[2] = {20, 34};
double total_revenue = 0.0;

sem_t ticket_counter, flavor_counter, flavor1, flavor2, flavor3, topping_counter, topping1, topping2, payment_counter;

void *process_customer(void *customer_id);

int main() {
    int num_customers;

    printf("\nEnter Number Of Customers [1-%d]: ", total_tickets);
    scanf("%d", &num_customers);
    printf("\n");

    if (num_customers > total_tickets || num_customers <= 0) {
        printf("\nInvalid Input!\n");
        return 0;
    }

    int customer_ids[num_customers];

    for (int i = 0; i < num_customers; i++) {
        customer_ids[i] = i + 100;
    }

    sem_init(&ticket_counter, 0, 1);

    sem_init(&flavor_counter, 0, 3);
    sem_init(&flavor1, 0, 1);
    sem_init(&flavor2, 0, 1);
    sem_init(&flavor3, 0, 1);

    sem_init(&topping_counter, 0, 2);
    sem_init(&topping1, 0, 1);
    sem_init(&topping2, 0, 1);

    sem_init(&payment_counter, 0, 1);

    pthread_t customers[num_customers];

    for (int i = 0; i < num_customers; i++) {
        pthread_create(&customers[i], NULL, process_customer, (void*)&customer_ids[i]);
    }

    for (int i = 0; i < num_customers; i++) {
        pthread_join(customers[i], NULL);
    }

    printf("\nBusiness Journal - At Closing\n");
    printf("Number Of Customers: %d", num_customers);
    printf("\nRevenue Generated: $%f", total_revenue);
    printf("\nTickets Remaining: %d\n", total_tickets);

    sem_destroy(&ticket_counter);
    sem_destroy(&flavor_counter);
    sem_destroy(&flavor1);
    sem_destroy(&flavor2);
    sem_destroy(&flavor3);
    sem_destroy(&topping_counter);
    sem_destroy(&topping1);
    sem_destroy(&topping2);
    sem_destroy(&payment_counter);

    return 0;
}

void *process_customer(void *customer_id) {
    int id = *(int*)customer_id;
    double bill = 0.0;

    sem_wait(&ticket_counter);

    if (total_tickets <= 0) {
        printf("\nCustomer[%d]: Leaving Shop. [Reason]: Tickets Finished\n", id);
        syscall(333, "Leaving Shop. [Reason]: Tickets Finished\n", id);
        sleep(1);
        pthread_exit(NULL);
    }

    total_tickets--;
    printf("Customer[%d] Got Ticket.\n", id);
    syscall(333, "Got Ticket.\n", id);

    sem_post(&ticket_counter);

    sleep(2);

    sem_wait(&flavor_counter);

    if (flavors[0] <= 0 && flavors[1] <= 0 && flavors[2] <= 0) {
        printf("\nCustomer[%d]: Leaving Shop. [Reason]: Flavors Finished\n", id);
        syscall(333, "Leaving Shop. [Reason]: Flavors Finished\n", id);
        sleep(1);
        pthread_exit(NULL);
    } else {
        sem_wait(&flavor1);

        if (flavors[0] > 0) {
            flavors[0]--;
            bill += FLAVOR_1_PRICE;
            printf("\nCustomer[%d]: Got Flavor[0].\n", id);
            syscall(333, "Got Flavor[0].\n", id);
            sleep(1);
        }

        sem_post(&flavor1);

        sem_wait(&flavor2);

        if (flavors[1] > 0) {
            flavors[1]--;
            bill += FLAVOR_2_PRICE;
            printf("Customer[%d]: Got Flavor[1].\n", id);
            syscall(333, "Got Flavor[1].\n", id);
            sleep(1);
        }

        sem_post(&flavor2);

        sem_wait(&flavor3);

        if (flavors[2] > 0) {
            flavors[2]--;
            bill += FLAVOR_3_PRICE;
            printf("Customer[%d]: Got Flavor[2].\n", id);
            syscall(333, "Got Flavor[2].\n", id);
            sleep(1);
        }

        sem_post(&flavor3);

        if (bill == 0.0) {
            printf("\nCustomer[%d]: Leaving Shop. [Reason]: Flavors Finished\n", id);
            syscall(333, "Leaving Shop. [Reason]: Flavors Finished\n", id);
            sleep(1);
            pthread_exit(NULL);
        }
    }

    printf("\nCustomer[%d]: Got Flavor(s). Leaving Flavor Counter\n", id);
    syscall(333, "Got Flavor(s). Leaving Flavor Counter\n", id);

    sem_post(&flavor_counter);

    sem_wait(&topping_counter);

    sem_wait(&topping1);

    if (toppings[0] > 0) {
        toppings[0]--;
        bill += TOPPING_1_PRICE;
    }

    sem_post(&topping1);

    sem_wait(&topping2);

    if (toppings[1] > 0) {
        toppings[1]--;
        bill += TOPPING_2_PRICE;
    }

    sem_post(&topping2);

    printf("\nCustomer[%d]: Leaving Topping Counter.\n", id);
    syscall(333, "Leaving Topping Counter.\n", id);

    sem_post(&topping_counter);

    sleep(2);

    sem_wait(&payment_counter);

    total_revenue += bill;

    printf("\nCustomer[%d]: Billed: $%f.\n", id, bill);
    char log_msg[100];
    sprintf(log_msg, "Billed: $%f.\n", bill);
    syscall(333, log_msg, id);

    sem_post(&payment_counter);

    sleep(2);

    printf("\nCustomer[%d]: Leaving Ice-Cream Shop.\n\n", id);
    syscall(333, "Leaving Ice-Cream Shop.\n\n", id);

    return NULL;
}


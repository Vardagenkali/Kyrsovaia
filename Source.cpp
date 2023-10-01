#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <limits>

std::wstring g_site;
std::wstring g_login;
std::wstring g_password;

std::wstring g_site_del;
void DisplayPasswords(HWND hPasswordList);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#pragma region Generator
std::wstring GeneratePassword(int length)
{
    std::wstring password;
    const wchar_t* charset = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+{}[]|\\:;\"'<>,.?/";

    for (int i = 0; i < length; ++i)
    {
        int index = rand() % wcslen(charset);
        password.push_back(charset[index]);
    }

    return password;
}

std::wstring WritePasswordToFile(const std::wstring& site, const std::wstring& login, const std::wstring& password)
{
    std::wofstream outFile(L"Password.txt", std::ios::app);
    if (outFile.is_open())
    {
        outFile << L"Site: " << site << std::endl;
        outFile << L"Login: " << login << std::endl;
        outFile << L"Password: " << password << std::endl;
        outFile << std::endl;
        outFile.close();
    }

    return password;
}

#pragma endregion

#pragma region Delete
void DeletePasswordFromFile(const std::wstring& site_del)
{
    std::wifstream inFile("Password.txt");
    if (!inFile)
    {
        std::wcerr << L"Error opening input file." << std::endl;
        return;
    }

    std::wofstream tempFile("TempPassword.txt");
    if (!tempFile)
    {
        std::wcerr << L"Error opening temporary output file." << std::endl;
        inFile.close();
        return;
    }

    std::wstring line;
    bool found = false;

    while (std::getline(inFile, line))
    {

        if (line.find(L"Site:") == 0 && line.substr(6) == site_del)
        {
            found = true;
            std::wstring login, password;
            std::getline(inFile, login);
            std::getline(inFile, password);
            continue;
        }

        tempFile << line << std::endl;
    }

    inFile.close();
    tempFile.close();

    if (!found)
    {
        std::wcout << L"No matching password entries found." << std::endl;
        return;
    }

    if (std::remove("Password.txt") == 0 && std::rename("TempPassword.txt", "Password.txt") == 0)
    {
        std::wcout << L"Matching password entries deleted successfully." << std::endl;
    }
    else
    {
        std::wcerr << L"Error deleting or renaming files." << std::endl;
    }
}
#pragma endregion


#pragma region Edit
void ReplacePassword(const std::wstring& site_edit, const std::wstring& login_edit, const std::wstring& new_password)
{
    std::wifstream inFile("Password.txt");
    if (!inFile)
    {
        std::wcerr << L"Error opening input file." << std::endl;
        return;
    }

    std::wofstream tempFile("TempPassword.txt");
    if (!tempFile)
    {
        std::wcerr << L"Error opening temporary output file." << std::endl;
        inFile.close();
        return;
    }

    std::wstring line;
    bool found = false;

    while (std::getline(inFile, line))
    {
        if (line.find(L"Site:") == 0 && line.substr(6) == site_edit)
        {
            found = true;
            tempFile << line << std::endl; // Сохраняем Site
            std::wstring login, password;
            std::getline(inFile, login); // Считываем Login
            std::getline(inFile, password); // Считываем старый пароль, но не записываем его в новый файл
            tempFile << L"Login: " << login_edit << std::endl; // Записываем новый Login
            tempFile << L"Password: " << new_password << std::endl; // Записываем новый пароль
        }
        else
        {
            tempFile << line << std::endl; // Сохраняем остальные строки без изменений
        }
    }

    inFile.close();
    tempFile.close();

    if (!found)
    {
        std::wcout << L"No matching password entry found for site: " << site_edit << std::endl;
        return;
    }

    if (std::remove("Password.txt") == 0 && std::rename("TempPassword.txt", "Password.txt") == 0)
    {
        std::wcout << L"Password updated successfully." << std::endl;
    }
    else
    {
        std::wcerr << L"Error updating password." << std::endl;
    }
}


#pragma endregion


#pragma region Check
void DisplayPasswords(HWND hWndList)
{
    std::wifstream inFile("Password.txt");
    if (!inFile)
    {
        std::wcerr << L"Error opening input file." << std::endl;
        return;
    }

    std::wstring line;
    std::wstring passwordEntry;

    while (std::getline(inFile, line))
    {
        passwordEntry += line + L"\r\n "; 
        if (line.empty())
        {

            SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)passwordEntry.c_str());
            passwordEntry.clear(); 
        }
    }

    inFile.close();
}

#pragma endregion



int main()
{
    DeletePasswordFromFile(L"Site: google.com");
    return 0;
}

int CALLBACK wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR szCmdLine, int nCmdShow)
{
    MSG msg{};
    HWND hwnd{};
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = hInst;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"APP";
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassEx(&wc)) return EXIT_FAILURE;

    if (hwnd = CreateWindow(wc.lpszClassName, L"Password manager", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr, wc.hInstance, nullptr); hwnd == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}


static std::pair<bool, HWND> AddWindow(const std::wstring&& winClass, const std::wstring&& title, HWND hParentWnd, const WNDPROC callback) {

    UnregisterClass(winClass.c_str(), GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND hWindow{};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpfnWndProc = callback; 
    wc.lpszClassName = winClass.c_str(); 
    wc.style = CS_VREDRAW | CS_HREDRAW;

    const auto create_window = [&hWindow, &title, &hParentWnd, &wc]() -> std::pair<bool, HWND> {
        if (hWindow = CreateWindow(wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, 0, 0, 300, 300, hParentWnd, nullptr, nullptr, nullptr); !hWindow)
            return { false, nullptr };

        ShowWindow(hWindow, SW_SHOWDEFAULT);
        UpdateWindow(hWindow);
        return { true, hWindow };
        };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

HFONT CreateCustomFont()
{
    return CreateFont(
        30, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"robot"
    );
}

HWND CreateTextControl(HWND parent, LPCWSTR text1, int x, int y, int width, int height, int id)
{
    HFONT hFont = CreateCustomFont();
    HWND text = CreateWindow(
        L"static", text1,
        WS_CHILD | WS_VISIBLE | SS_CENTER | WS_CLIPCHILDREN,
        x, y, width, height,
        parent, reinterpret_cast<HMENU>(id), nullptr, nullptr
    );
    SetClassLong(text, GCL_HBRBACKGROUND, reinterpret_cast<LONG>(GetStockObject(WHITE_BRUSH)));

    SendMessage(text, WM_SETFONT, WPARAM(hFont), TRUE);
    return text;
}

HWND CreateEditControl(HWND parent, LPCWSTR text1, int x, int y, int width, int height, int id)
{
    HFONT hFont = CreateCustomFont();
    HWND edit = CreateWindow(
        L"EDIT", 
        text1,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER, 
        x, y, width, height,
        parent, reinterpret_cast<HMENU>(id), nullptr, nullptr
    );
    SetClassLong(edit, GCL_HBRBACKGROUND, reinterpret_cast<LONG>(GetStockObject(WHITE_BRUSH)));

    SendMessage(edit, WM_SETFONT, WPARAM(hFont), TRUE);
    return edit;
}

HWND CreateButton(HWND parent, LPCWSTR text, int x, int y, int width, int height, int id, COLORREF bgColor, COLORREF textColor)
{
    HWND button = CreateWindow(
        L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, width, height,
        parent, reinterpret_cast<HMENU>(id), nullptr, nullptr
    );
    SendMessage(button, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    SetTextColor(GetDC(button), textColor);

    return button;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hChildWnd{};
    switch (uMsg)
    {
    case WM_CREATE:{
        CreateTextControl(hWnd, L"Password Manager", 600,200,300,50, 10 );
        CreateButton(hWnd, L"Generate Password", 0, 400, 300, 50, 0, RGB(0, 0, 255), RGB(255, 255, 255)); // Синий текст
        CreateButton(hWnd, L"DELETE",  400, 400, 300, 50, 1337, RGB(255, 0, 0), RGB(255, 255, 255)); // Красный текст
        CreateButton(hWnd, L"Edit password", 800, 400, 300, 50, 1, RGB(255, 255, 0), RGB(0, 0, 0)); // Желтый текст
        CreateButton(hWnd, L"Check passwords", 1200, 400, 300, 50, 2, RGB(0, 128, 0), RGB(0, 0, 0)); // Зеленый текст
        return 0;
    }

    case WM_CTLCOLORBTN:{
        HDC hdcButton = (HDC)wParam;
        HWND hButton = (HWND)lParam;
        int buttonId = GetDlgCtrlID(hButton);
        switch (buttonId)
        {
        case 0: 
            SetBkColor(hdcButton, RGB(0, 0, 255)); 
            return (LRESULT)CreateSolidBrush(RGB(0, 0, 255));
        case 1337: 
            SetBkColor(hdcButton, RGB(255, 0, 0)); 
            return (LRESULT)CreateSolidBrush(RGB(255, 0, 0));
        case 1:
            SetBkColor(hdcButton, RGB(255, 255, 0)); 
            return (LRESULT)CreateSolidBrush(RGB(255, 255, 0));
        case 2: 
            SetBkColor(hdcButton, RGB(0, 128, 0)); 
            return (LRESULT)CreateSolidBrush(RGB(0, 128, 0));
        case 3:
            SetBkColor(hdcButton, RGB(0, 0, 0));
            return (LRESULT)CreateSolidBrush(RGB(0, 0, 0));
        }
        break;
    }
    case WM_CTLCOLORSTATIC:{
        case 10:
            HDC hdcStatic = (HDC)wParam;
            HWND hText = (HWND)lParam;

            SetTextColor(hdcStatic, RGB(0,0, 0));
            SetBkColor(hdcStatic, RGB(255, 255, 255));

            return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    case WM_COMMAND:
    {
        int buttonId = LOWORD(wParam);
#pragma region Объявляем переменные до использования в switch



        std::pair<bool, HWND> result; 
        bool flag;
        HWND hChild;

        std::pair<bool, HWND> result2; 
        bool flag2;
        HWND hChild2;

        std::pair<bool, HWND> result3; 
        bool flag3;
        HWND hChild3;

        std::pair<bool, HWND> result4; 
        bool flag4;
        HWND hChild4;
#pragma endregion

        switch (buttonId)
        {
#pragma region Генерация пароля

    case 0:
            if (hChildWnd) DestroyWindow(hChildWnd);
            result = AddWindow(L"Generate password", L"Generate password", hWnd, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT {
                switch (uMsg) {
                    case WM_CREATE:
                        {
                        CreateTextControl(hWnd, L"Generate password", 600, 100, 300, 50, 10);
                        CreateEditControl(hWnd, L"Введите название сайта", 100, 300, 500, 50, 102);
                        CreateEditControl(hWnd, L"ВВедите логин", 100, 400, 500, 50, 11);
                        CreateEditControl(hWnd, L"Введите сколько символов должен содержать пароль", 100, 500, 500, 50, 13);
                        CreateButton(hWnd, L"ACCEPT", 1000, 280, 600, 300, 90, RGB(255, 255, 255), RGB(0, 0, 0));
                        break;
                        }
                    case WM_COMMAND:
                    {
                        int buttonId = LOWORD(wParam);

                        switch (buttonId)
                        {
                        case 90:
                        {
                            HWND hSiteEdit = GetDlgItem(hWnd, 102); // Поле с названием сайта
                            HWND hLoginEdit = GetDlgItem(hWnd, 11); // Поле с логином
                            HWND hLengthEdit = GetDlgItem(hWnd, 13); // Поле с указанием длины пароля

                            // Получаем текст из полей
                            wchar_t site[256];
                            wchar_t login[256];
                            wchar_t lengthText[256];

                            GetWindowText(hSiteEdit, site, 256);
                            GetWindowText(hLoginEdit, login, 256);
                            GetWindowText(hLengthEdit, lengthText, 256);

                            // Преобразуем текст в числовое значение
                            int passwordLength = _wtoi(lengthText);

                            // Генерируем пароль и сохраняем его в глобальную переменную
                            g_password = GeneratePassword(passwordLength);

                            g_site = site;
                            g_login = login;

                            std::wstring generatedPassword = GeneratePassword(passwordLength);
                            WritePasswordToFile(g_site, g_login, generatedPassword);

                            if (!generatedPassword.empty())
                            {
                                std::wstring message = L"Site: " + g_site + L"\nLogin: " + login + L"\nPassword: " + g_password;
                                MessageBox(hWnd, message.c_str(), L"Password Information", MB_OK | MB_ICONINFORMATION);
                            }
                            else
                            {
                                MessageBox(hWnd, L"Failed to save the password to the file.", L"Error", MB_OK | MB_ICONERROR);
                            }

                            SetWindowText(hSiteEdit, L"");
                            SetWindowText(hLoginEdit, L"");
                            SetWindowText(hLengthEdit, L"");

                            break;
                        }
                        }
                        return 0;
                    }
                }
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
                });
            flag = result.first;
            hChild = result.second;
            hChildWnd = hChild;
            break;


#pragma endregion
#pragma region Удаление пароля

    case 1337:
            if (hChildWnd) DestroyWindow(hChildWnd);
            result2 = AddWindow(L"Delete Window", L"Delete Window", hWnd, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT {
                switch (uMsg) {
                case WM_CREATE:
                {
                    CreateTextControl(hWnd, L"Delete password", 600, 100, 300, 50, 10);
                    CreateEditControl(hWnd, L"Введите название сайта", 100, 300, 600, 200, 103);
                    CreateButton(hWnd, L"ACCEPT", 1000, 280, 400, 200, 91, RGB(255, 255, 255), RGB(0, 0, 0));
                    break;
                }

                case WM_COMMAND:
                {
                    int buttonId = LOWORD(wParam);

                    switch (buttonId)
                    {
                    case 91:
                    {
                        HWND hDelSiteEdit = GetDlgItem(hWnd, 103); // Поле с названием сайта

                        wchar_t site_del[256];

                        GetWindowText(hDelSiteEdit, site_del, 256);

                        // Вызываем функцию для удаления записей
                        DeletePasswordFromFile(site_del);

                        // Закрываем окно после удаления
                        DestroyWindow(hWnd);
                        break;

                    }

                    }

                    return 0;
                }
                }
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
                });
            flag2 = result2.first;
            hChild2 = result2.second;
            hChildWnd = hChild2;
            break;

#pragma endregion
#pragma region Изменение пароля

    case 1:
        if (hChildWnd) DestroyWindow(hChildWnd);
        result3 = AddWindow(L"Edit Window", L"Edit Window", hWnd, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT {
            switch (uMsg) {
            case WM_CREATE:
            {
                CreateTextControl(hWnd, L"Edit password", 600, 100, 300, 50, 10);
                CreateEditControl(hWnd, L"Введите название сайта", 100, 300, 500, 50, 1009);
                CreateEditControl(hWnd, L"Введите логин", 100, 400, 500, 50, 1007);
                CreateEditControl(hWnd, L"Тут покажется новый пароль", 100, 500, 500, 50, 1008);
                CreateButton(hWnd, L"ACCEPT", 1000, 280, 200, 100, 993, RGB(255, 255, 255), RGB(0, 0, 0));
                CreateButton(hWnd, L"Generate new password", 1000, 400, 200, 100, 983, RGB(255, 255, 255), RGB(0, 0, 0));
                break;
            }

            case WM_COMMAND:
            {
                int buttonId = LOWORD(wParam);

                switch (buttonId)
                {
                case 983: 
                {
                    HWND hEditPass = GetDlgItem(hWnd, 1008); // Поле с новым паролем

                    // Генерируем новый пароль
                    int passwordLength = 12; // Здесь можно установить желаемую длину пароля
                    std::wstring new_password = GeneratePassword(passwordLength);

                    SetWindowText(hEditPass, new_password.c_str()); // Устанавливаем новый пароль в поле ввода
                    break;
                }

                case 993: // Команда "ACCEPT" (применить изменения)
                {
                    HWND hEditSite = GetDlgItem(hWnd, 1009); // Поле с названием сайта
                    HWND hEditLogin = GetDlgItem(hWnd, 1007); // Поле с логином
                    HWND hEditPass = GetDlgItem(hWnd, 1008); // Поле с новым паролем

                    wchar_t site_edit[256];
                    wchar_t login_edit[256];
                    wchar_t pass_edit[256];

                    GetWindowText(hEditSite, site_edit, 256);
                    GetWindowText(hEditLogin, login_edit, 256);
                    GetWindowText(hEditPass, pass_edit, 256);

                    if (!site_edit[0] || !login_edit[0] || !pass_edit[0])
                    {
                        MessageBox(hWnd, L"Site name, login, or new password cannot be empty.", L"Error", MB_OK | MB_ICONERROR);
                    }
                    else
                    {
                        // Заменяем пароль и обновляем запись
                        ReplacePassword(site_edit, login_edit, pass_edit);

                        MessageBox(hWnd, L"Password updated successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
                    }

                    SetWindowText(hEditSite, L"");
                    SetWindowText(hEditLogin, L"");
                    SetWindowText(hEditPass, L"");

                    break;
                }
                }

                return 0;
            }
            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
            });
        flag3 = result3.first;
        hChild3 = result3.second;
        hChildWnd = hChild3;
        break;

#pragma endregion
#pragma region Показать пароли



    case 2:
    {
        if (hChildWnd) DestroyWindow(hChildWnd);
        result4 = AddWindow(L"Check Window", L"Check Window", hWnd, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT {
            switch (uMsg) {
            case WM_CREATE:
            {
                CreateTextControl(hWnd, L"Check Passwords", 600, 100, 300, 50, 10);
                HWND hPasswordList = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY,
                    100, 200, 1300, 500, hWnd, (HMENU)11, NULL, NULL);

                DisplayPasswords(hPasswordList); 
                break;
            }

            }
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
            });
        flag4 = result4.first;
        hChild4 = result4.second;
        hChildWnd = hChild4;
        break;
    }
#pragma endregion
    }
    break;
    }


    case WM_DESTROY:{
        PostQuitMessage(EXIT_SUCCESS);
        return 0;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


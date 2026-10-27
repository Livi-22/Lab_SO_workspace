#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

// Includem librariile specifice fiecarui sistem de operare
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

int main(int argc, char* argv[]) {
    // Mecanism pentru Windows: executabilul se apeleaza pe sine cu argumente
    // Daca primeste argumentul "child", stie ca este proces copil
    if (argc == 3 && std::string(argv[1]) == "child") {
        int secunde = std::stoi(argv[2]);
#ifdef _WIN32
        Sleep(secunde * 1000); // Sleep pe Windows foloseste milisecunde
#else
        sleep(secunde); // sleep pe Linux foloseste secunde
#endif
        return 0; // Copilul isi termina executia
    }

    std::cout << "Programul initial (parintele) a inceput si lanseaza copiii." << std::endl;
    int timpi_asteptare[] = {1, 2, 3};

#ifdef _WIN32
    // Abordarea pe Windows folosind Windows API (CreateProcess)
    std::vector<HANDLE> procese;
    
    for (int i = 0; i < 3; i++) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Construim comanda: executabilul curent + "child" + secunde
        std::string comanda = std::string(argv[0]) + " child " + std::to_string(timpi_asteptare[i]);
        char* cmdArgs = &comanda[0];

        if (CreateProcess(NULL, cmdArgs, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            procese.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
            std::cout << "- Procesul " << i + 1 << " a fost creat (Windows). Asteapta " << timpi_asteptare[i] << " secunde." << std::endl;
        }
    }
    
    // Programul parinte asteapta pana se termina absolut toate procesele copil
    WaitForMultipleObjects(procese.size(), procese.data(), TRUE, INFINITE);
    
    for (HANDLE h : procese) {
        CloseHandle(h);
    }

#else
    // Abordarea pe Linux folosind clonarea cu fork()
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            std::cerr << "Eroare la crearea procesului." << std::endl;
        } else if (pid == 0) {
            // Ne aflam fix in memoria procesului copil
            std::cout << "- Procesul " << i + 1 << " a fost creat (Linux). Asteapta " << timpi_asteptare[i] << " secunde." << std::endl;
            sleep(timpi_asteptare[i]);
            return 0; // Iesim din copil pentru a nu executa codul parintelui
        }
    }
    
    // Programul parinte asteapta terminarea tuturor proceselor copil
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }
#endif

    // Mesajul final se afiseaza doar o singura data de catre procesul original
    std::cout << "Toate procesele s-au terminat. Programul initial se inchide." << std::endl;
    return 0;
}
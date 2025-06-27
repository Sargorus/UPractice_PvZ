#include <vector>

#include <iostream>
#include <thread>
#include <chrono>
#include <conio.h> // Для функций _kbhit() и _getch()
#include <windows.h> // Для работы с консолью Windows
#include <string>
#include <cctype> // для std::toupper

#include "Entities.h"

bool SaveGame(std::vector<Entity*> entities, Level level);
constexpr std::chrono::milliseconds INPUT_WAIT_TIME(375);


int ColumnLetterToNumber(char letter) {
    // Предполагается, что буква в диапазоне A-Z
    return std::toupper(letter) - 'A' + 1;
}

void ParseCoordinates(const std::string& coord, int& colIndex, int& rowIndex) {
    // Предположим, что формат: одна или несколько букв + число
    size_t i = 0;
    // Обработка буквенного столбца
    std::string colPart;
    while (i < coord.size() && std::isalpha(coord[i])) {
        colPart += coord[i];
        ++i;
    }

    // Обработка номера строки
    std::string rowPart = coord.substr(i);

    // Преобразуем буквы в число
    colIndex = 0;
    for (char c : colPart) {
        colIndex = colIndex * 26 + ColumnLetterToNumber(c);
    }

    // Преобразуем строку в число
    rowIndex = std::stoi(rowPart);
}

void SplitString(const std::string& input, std::string& part1, std::string& part2) {
    size_t pos = input.find('-'); //A2-1
    if (pos != std::string::npos) {
        part1 = input.substr(0, pos);        // часть до '-'
        part2 = input.substr(pos + 1);       // часть после '-'
    }
    else {
        // если разделитель не найден, можно обработать ситуацию
        part1 = input;
        part2 = "\0";
    }
}

bool FindExist(std::vector<Entity*>& entities, int x, int y) {
    for (Entity* e : entities) {
        if (e->X == x && e->Y == y) {
            if (e->type != EntityType::MISSILE || e->type != EntityType::ZOMBIE) {
                return false;
            }
            return true;
        }
    }
    return false;
}

void CreatePlant(std::string data, std::vector<Entity*>& entities, int house_offset, std::vector<int> linesY, int columns, int& summa) {
    std::string coord, s_type;
    SplitString(data, coord, s_type);

    if (s_type == "\0") {
        return;
    }

    int x, y;
    char type = s_type[0];
    ParseCoordinates(coord, x, y);

    if (x > columns) {
        x = columns;
    }
    x = x * 2 + house_offset - 1;

    if (y >= linesY.size()) {
        y = linesY.size();
    }
    y = linesY[y - 1];

    if (FindExist(entities, x, y)) {
        return;
    }

    Plant* plant;

    switch (type)
    {
    case 's':
    case 'S':
        plant = new SunflowerPlant(x, y);
        break;
    case 'p':
    case 'P':
        plant = new PeasfirePlant(x, y);
        break;
    case 'n':
    case 'N':
        plant = new NutwallPlant(x, y);
        break;
    case 'c':
    case 'C':
        plant = new CherryboomPlant(x, y);
        break;
    default:
        return;
    }
    if (summa >= plant->Cost) {
        entities.push_back(plant);
        summa -= plant->Cost;
    }
}

void clearInputBuffer() {
    while (_kbhit()) {
        _getch();
    }
}

bool GetInput(HANDLE hStdin, DWORD originalMode, std::vector<Entity*>& entities, int house_offset, std::vector<int> linesY, int columns, int& summa, Level level) {
    // === 1. Фаза выполнения движка (блокировка ввода) ===
    // Отключаем отображение ввода и очищаем буфер
    SetConsoleMode(hStdin, originalMode & ~ENABLE_ECHO_INPUT);
    clearInputBuffer();

    // === 2. Фаза ожидания ввода (1 секунда) ===
    // Включаем обработку ввода
    SetConsoleMode(hStdin, originalMode | ENABLE_ECHO_INPUT);
    auto inputStart = std::chrono::steady_clock::now();
    bool hasInput = false;

    // Ожидаем ввод в течение 1 секунды
    while (std::chrono::steady_clock::now() - inputStart < INPUT_WAIT_TIME) {
        if (_kbhit()) {
            hasInput = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // === 3. Обработка результатов ввода ===
    if (hasInput) {
        // Считываем всю строку до Enter
        std::string input;
        char ch;
        while ((ch = _getch()) != '\r') { // \r - символ Enter в Windows
            if (ch == '\b' && !input.empty()) { // Обработка Backspace
                input.pop_back();
                std::cout << "\b \b"; // Удаляем символ из консоли
            }
            else if (ch >= 32 && ch <= 126) { // Печатные ASCII символы
                input += ch;
                std::cout << ch;
            }
        }
        if (input == "stop" || input == "STOP") {
            SaveGame(entities, level);
            return true;
        }
        CreatePlant(input, entities, house_offset, linesY, columns, summa);
    }
    return false;
}
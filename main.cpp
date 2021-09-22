#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>

#include <unistd.h>

void printHelp() {
    std::cout << "\n sort0 is a great sorting application for testing the author by CFT\n"
              << "\n Maximum files to compare = 256. The integer values in files can be from -9 223 372 036 854 775 808 to 9 223 372 036 854 775 807\n"
              << "\n Usage:\n"
              << "   sort0 [sort direction] <datatype> <outFile> <inFile 1> [inFile 2] ... [inFile N]\n"
              << "\n Possible values for [sort direction]:\n"
              << "   -a \t ascending sort\n"
              << "   -d \t descending sort\n"
              << "\n Possible values for <datatype>:\n"
              << "   -s \t sort strings\n"
              << "   -i \t sort integers\n" << std::endl;
}

struct SortParams {
    bool direction = 0; //направление сортировки 0 - по возрастанию (по умолчанию), 1 - по убыванию
    bool datatype; //тип данных сортировки 0 - числа, 1 - строки
    std::string out_file; //имя выходного файла
    std::vector<std::string> if_name; //вектор с входными файлами
};

bool doSortInt(const std::string &out, const std::string &in1, const std::string &in2, bool direction);
bool doSortString(const std::string &out, const std::string &in1, const std::string &in2, bool direction);

int main(int argc, char *argv[])
{
    //если аргументов командной строки недостаточно для работы, выводим help.
    if (argc < 4) {
        printHelp();
        return 0;
    }

    SortParams sortp;
    int16_t curr_argv = 1; //текущий элемент массива аргументов командной строки
    //заполняем правила сортировки в sortp
    if (std::string(argv[curr_argv]) == "-a") {
        curr_argv++;
    } else if (std::string(argv[curr_argv]) == "-d") {
        sortp.direction = 1;
        curr_argv++;
    }
    if (std::string(argv[curr_argv]) == "-s") {
        sortp.datatype = 1;
        curr_argv++;
    } else if (std::string(argv[curr_argv]) == "-i") {
        sortp.datatype = 0;
        curr_argv++;
    } else {
        printHelp();
        return 0;
    }
    //проверяем существование файла для записи результата сортировки
    std::ifstream temp_file (argv[curr_argv]); //поток для проверки возможности открытия файлов
    if (temp_file.is_open()) {
        temp_file.close();
        std::cout << "File \"" << argv[curr_argv]
                  << "\" already exists. Enter \"yes\" and then press \"Enter\" for erase all data in \""
                  << argv[curr_argv] << "\" : ";
        std::string user_y;
        std::cin >> user_y;
        if (user_y == "yes") { //если юзер согласен удалить содержимое файла, открываем его на запись
            sortp.out_file = argv[curr_argv];
            std::cout << "The file \"" << argv[curr_argv] << "\" contents will be erased" << std::endl;
        } else {
            std::cout << "\n Abort sorting by user" << std::endl;
            return 0;
        }
    } else { //если файл не существует, сохраняем его имя
        sortp.out_file = argv[curr_argv];
    }
    curr_argv++;
    //заносим имена удачно открытых входных файлов в sortp.in_file
    for (uint8_t i = curr_argv; i < argc; i++) {
        temp_file.open(argv[i]);
       if (temp_file.is_open()) {
            sortp.if_name.push_back(argv[i]);
        } else {
            std::cout << "Can't open file \"" << argv[i] << "\"" << std::endl;
        }
        temp_file.close();
    }
    if (sortp.if_name.size() == 0) { //если ничего открыть не удалось, выходим из программы
        std::cout << "No files to sort" << std::endl;
        return 0;
    }

    std::string out_temp_file_name; //имя временного файла для хранения промежуточных сортировок
    std::vector<std::string> temp_files_name; //вектор для имен временных файлов (для удаления по завершению сортировки)
    uint8_t counter = 0; //счетчик для имен файлов
    while (sortp.if_name.size() > 1) { //There can be only one
        if (sortp.if_name.size() == 2) {
            out_temp_file_name = sortp.out_file;
        } else {
            out_temp_file_name = "temp" + std::to_string(counter); //создаём имена временных файлов
            temp_files_name.push_back(out_temp_file_name); //сохраняем имена созданных файлов
        }
        sortp.if_name.insert(sortp.if_name.begin(), out_temp_file_name); //вставляем имя временного файла в начало вектора
        if (sortp.datatype) {
            if (!doSortString(sortp.if_name.front(), //запускаем сортировку. читаем из 2-х последних файлов в векторе, пишем в первый
               sortp.if_name.at(sortp.if_name.size() - 2), //[sortp.if_name.end()],
               sortp.if_name.back(),
               sortp.direction))
            {
                std::cout << "File I/O fatal error" << std::endl;
                return 0;
            }
        } else {
            if (!doSortInt(sortp.if_name.front(), //запускаем сортировку. читаем из 2-х последних файлов в векторе, пишем в первый
               sortp.if_name.at(sortp.if_name.size() - 2), //[sortp.if_name.end()],
               sortp.if_name.back(),
               sortp.direction))
            {
                std::cout << "File I/O fatal error" << std::endl;
                return 0;
            }
        }
        sortp.if_name.pop_back(); //удаляем 2 последние записи
        sortp.if_name.pop_back();
        counter++;
    }
    for (const std::string &s : temp_files_name) { //удаляем временные файлы
        std::remove(s.c_str());
    }
    std::cout << "\n All done! Thanks for watching and sorry for my inglish)\n" << std::endl;
    return 0;
}

bool doSortString(const std::string &out, const std::string &in1, const std::string &in2, bool direction) {
    std::ofstream ofile (out);
    if (!ofile) {
        return 0;
    }
    std::ifstream ifile1 (in1);
    if (!ifile1) {
        ofile.close();
        return 0;
    }
    std::ifstream ifile2 (in2);
    if (!ifile2) {
        ofile.close();
        ifile1.close();
        return 0;
    }
    std::string prev_str, f1_str, f2_str; //строки для считывания данных из файлов. prev_str - хранит последнее записанное значение
    std::getline(ifile2, f2_str); //считываем первую строку из 2-го файла
    if (!direction) { //выбор направления сортировки
        prev_str = "";
        while (std::getline(ifile1, f1_str)) { //читает первый файл до конца
            while (!ifile2.eof() && (f2_str.compare(f1_str) < 0)) { //проверяем eof на втором файле и сравниваем строки
                if (f2_str.compare(prev_str) < 0) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_str = f2_str; //сохраняем последнее значение в prev_str
                    ofile << f2_str << std::endl; //пишем в выходной файл
                }
                std::getline(ifile2, f2_str);
            }
            if (f1_str.compare(prev_str) < 0) { //проверка на правильность сортировки входных данных
                std::cout << "File \"" << in1 << "\" is not sorted correctly. One line skiped." << std::endl;
            } else {
                prev_str = f1_str;
                ofile << f1_str << std::endl;
            }
        }
        if (!ifile2.eof()) {
            do { //дочитываем второй файл
                if (f2_str.compare(prev_str) < 0) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_str = f2_str; //сохраняем последнее значение в prev_str
                    ofile << f2_str << std::endl; //пишем в выходной файл
                }
            } while (std::getline(ifile2, f2_str));
        }
    } else {
        bool prev_str_flag = 1; //флаг инициализации prev_str (костыль)
        while (std::getline(ifile1, f1_str)) { //читает первый файл до конца
            if (prev_str_flag) { //кривая инициализация начального значения prev_str
                if (f2_str.compare(f1_str) > 0) {
                    prev_str = f2_str;
                } else {
                    prev_str = f1_str;
                }
                prev_str_flag = 0;
            }
            while (!ifile2.eof() && (f2_str.compare(f1_str) > 0)) { //проверяем eof на втором файле и сравниваем строки
                if (f2_str.compare(prev_str) > 0) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_str = f2_str; //сохраняем последнее значение в prev_str
                    ofile << f2_str << std::endl; //пишем в выходной файл
                }
                std::getline(ifile2, f2_str);
            }
            if (f1_str.compare(prev_str) > 0) { //проверка на правильность сортировки входных данных
                std::cout << "File \"" << in1 << "\" is not sorted correctly. One line skiped." << std::endl;
            } else {
                prev_str = f1_str;
                ofile << f1_str << std::endl;
            }
        }
        if (!ifile2.eof()) {
            do { //дочитываем второй файл
                if (f2_str.compare(prev_str) > 0) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_str = f2_str; //сохраняем последнее значение в prev_str
                    ofile << f2_str << std::endl; //пишем в выходной файл
                }
            } while (std::getline(ifile2, f2_str));
        }
    }
    ofile.close();
    ifile1.close();
    ifile2.close();
    return 1;
}

bool doSortInt(const std::string &out, const std::string &in1, const std::string &in2, bool direction) {
    std::ofstream ofile (out);
    if (!ofile) {
        return 0;
    }
    std::ifstream ifile1 (in1);
    if (!ifile1) {
        ofile.close();
        return 0;
    }
    std::ifstream ifile2 (in2);
    if (!ifile2) {
        ofile.close();
        ifile1.close();
        return 0;
    }
    signed long long prev_int, f1_int, f2_int; //переменные для считывания данных из файлов. prev_int - хранит последнее записанное значение
    if (!ifile2.eof()) {
        ifile2 >> f2_int; //считываем первую строку из 2-го файла
    }
    if (!direction) { //выбор направления сортировки
        prev_int = std::numeric_limits<signed long long>::min();
        while (ifile1 >> f1_int) { //читает первый файл до конца
            while (!ifile2.eof() && (f2_int < f1_int)) { //проверяем eof на втором файле и сравниваем строки
                if (f2_int < prev_int) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_int = f2_int; //сохраняем последнее значение в prev_str
                    ofile << f2_int << std::endl; //пишем в выходной файл
                }
                ifile2 >> f2_int; //читаем следующее число
            }
            if (f1_int < prev_int) { //проверка на правильность сортировки входных данных
                std::cout << "File \"" << in1 << "\" is not sorted correctly. One line skiped." << std::endl;
            } else {
                prev_int = f1_int;
                ofile << f1_int << std::endl;
            }
        }
        if (!ifile2.eof()) {
            do { //дочитываем второй файл
                if (f2_int < prev_int) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_int = f2_int; //сохраняем последнее значение в prev_str
                    ofile << f2_int << std::endl; //пишем в выходной файл
                }
            } while (ifile2 >> f2_int);
        }
    } else {
        prev_int = std::numeric_limits<signed long long>::max();
        while (ifile1 >> f1_int) { //читает первый файл до конца
            while (!ifile2.eof() && (f2_int > f1_int)) { //проверяем eof на втором файле и сравниваем строки
                if (f2_int > prev_int) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_int = f2_int; //сохраняем последнее значение в prev_str
                    ofile << f2_int << std::endl; //пишем в выходной файл
                }
                ifile2 >> f2_int; //читаем следующее число
            }
            if (f1_int > prev_int) { //проверка на правильность сортировки входных данных
                std::cout << "File \"" << in1 << "\" is not sorted correctly. One line skiped." << std::endl;
            } else {
                prev_int = f1_int;
                ofile << f1_int << std::endl;
            }
        }
        if (!ifile2.eof()) {
            do { //дочитываем второй файл
                if (f2_int > prev_int) { //проверка на правильность сортировки входных данных
                    std::cout << "File \"" << in2 << "\" is not sorted correctly. One line skiped." << std::endl;
                } else { //если сортировка верная
                    prev_int = f2_int; //сохраняем последнее значение в prev_str
                    ofile << f2_int << std::endl; //пишем в выходной файл
                }
            } while (ifile2 >> f2_int);
        }
    }
    ofile.close();
    ifile1.close();
    ifile2.close();
    return 1;
}


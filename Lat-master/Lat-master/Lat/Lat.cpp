
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <iostream>
#include <locale.h>
#include <windows.h>
#include <cstdlib> 
#include <atlstr.h>
#include <string>
#include <fstream>
#include <json.hpp>
#include <vector>

#include <algorithm>
#include <filesystem>
#include "pugixml.hpp"
#include "zip.h"
#include "unzip.h"
#include <thread>
#include <mutex>
#include <chrono>

#include <sha.h>

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace std;
//Директория существует (да или нет)
bool fileExists(string filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}
//Проверка строки на число
bool is_number(const string& s)
{
    return !s.empty() && find_if(s.begin(),
        s.end(), [](unsigned char c) { return !isdigit(c); }) == s.end();
}
//Проверка на существование директории
bool dirExists(string dirName_in)
{
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;
    return false;
}
//Вывод информации о дисках
void GetInfo()
{
    int n;
    bool Flag;
    DWORD dr = GetLogicalDrives();
    for (int x = 0; x < 26; x++)
    {
        n = ((dr >> x) & 1);
        if (n)
        {
            string DiskName(CT2CA(CString(TCHAR(65 + x))));
            string lPath = DiskName + ":\\";
            WORD OldErrorMode;
            OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
            bool ready = dirExists(lPath);
            SetErrorMode(OldErrorMode);
            if (ready)
            {
                cout << "Имя диска: ";
                cout << lPath;
                UINT drive_type = GetDriveTypeA(lPath.c_str());
                cout << "\nТип диска: ";
                if (drive_type == DRIVE_REMOVABLE) cout << "REMOVABLE";
                else if (drive_type == DRIVE_FIXED)     cout << "HARD DISK";
                else if (drive_type == DRIVE_REMOTE)    cout << "REMOTE";
                else if (drive_type == DRIVE_CDROM)     cout << "CD-ROM";
                else if (drive_type == DRIVE_RAMDISK)   cout << "RAMDISK";
                else cout << "Unknown Type\n";
                unsigned __int64 FreeBytesAvailable;
                unsigned __int64 TotalNumberOfBytes;
                unsigned __int64 TotalNumberOfFreeBytes;
                wstring temp = wstring(lPath.begin(), lPath.end());
                LPCWSTR lLPCWSTRPath = temp.c_str();
                Flag = ::GetDiskFreeSpaceEx(lLPCWSTRPath,
                    (PULARGE_INTEGER)&FreeBytesAvailable,
                    (PULARGE_INTEGER)&TotalNumberOfBytes,
                    (PULARGE_INTEGER)&TotalNumberOfFreeBytes);
                if (Flag)
                {
                    __int64 FreeGiga = TotalNumberOfFreeBytes / 1024 / 1024 / 1024;
                    __int64 TotalGiga = TotalNumberOfBytes / 1024 / 1024 / 1024;
                    cout << "\nСвободно на диске: " + to_string(FreeGiga) + " ГБ\n";
                    cout << "Всего на диске: " + to_string(TotalGiga) + " ГБ\n";
                }
                else cout << "Ошибка в GetDiskFreeSpaceEx\n";
                USES_CONVERSION_EX;
                LPWSTR afjha = A2W_EX(lPath.c_str(), lPath.length());
                _TCHAR FileSysteNameT[32] = _T("");
                Flag = GetVolumeInformation(afjha, NULL, NULL, NULL, NULL, NULL, FileSysteNameT, 32);
                wstring FileSysteNameW(&FileSysteNameT[0]);
                string FileSysteNameS(FileSysteNameW.begin(), FileSysteNameW.end());
                if (Flag)   cout << "Файловая система: " + FileSysteNameS + "\n";
                else        cout << "Ошибка в GetVolumeInformation\n";
            }
            cout << endl;
        }

    }

}
//Работа с файлами
class FileHandler {
public:
    static void File_cycle() {
        int endc = 0;
        string choice = "";
        while (endc == 0) {
            cout << "[Работа с файлами] Выберите необходимую функцию:\n";
            cout << "1. Информация о текущих txt файлах в рабочей директории\n";
            cout << "2. Чтение файла\n";
            cout << "3. Создание файла\n";
            cout << "4. Запись данных в файл\n";
            cout << "5. Удаление файла\n";
            cout << "6. Выход из работы с файлами\n";
            cout << "Ваш выбор: ";
            getline(cin, choice);
            cout << endl;
            if ((is_number(choice) == 0) or choice == "")
            {
                cout << "Введено неправильное значение. Повторите.\n\n";
                continue;
            }
            int intchoice = stoi(choice);
            switch (intchoice)
            {
            case 1:
                DirectoryFiles();
                break;
            case 2:
                ReadFile();
                break;
            case 3:
                CreateFile();
                break;
            case 4:
                RedactFile();
                break;
            case 5:
                DeleteFile();
                break;
            case 6:
                endc = 1;
                break;
            default:
                cout << "Неизвестная команда";
                break;
            }
            cout << endl;
        }
    }
    static void DirectoryFiles() {
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/txt_files" })
            std::cout << entry.path().filename() << std::endl;
    }
    static void ReadFile() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для чтения: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        ifstream fin("txt_files/" + filename + ".txt");
        if (fin) {
            getline(fin, text);
            fin >> text;
            cout << "Текст в файле " + filename + ".txt :\n";
            cout << text;
            fin.close();
        }
        else {
            cout << "Файла нет либо его нельзя открыть\n";
            return;
        }
    }
    static void CreateFile() {
        string filename = "";
        string text = "";
        cout << "Введите имя создаваемого файла: ";
        getline(cin, filename);
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (fileExists("txt_files/" + filename + ".txt") == 0) {
            ofstream fin("txt_files/" + filename + ".txt");
            cout << "Что добавить в файл:\n";
            getline(cin, text);
            cout << endl;
            if (text == "")
            {
                cout << "Введено неправильное значение.\n\n";
                return;
            }
            fin << text;
            cout << "Файл создан\n";
            fin.close();
        }
        else
        {
            cout << "Файл с таким именем уже существует\n";
            return;
        }
    }
    static void RedactFile() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для редактирования: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        ifstream fin("txt_files/" + filename + ".txt");
        if (fin) {
            getline(fin, text);
            fin >> text;
            cout << "Текст в файле " + filename + ".txt :\n";
            cout << text;
            fin.close();
        }
        else
        {
            cout << "Файла нет либо его нельзя открыть\n";
            return;
        }
        cout << "Что добавить в файл:\n";
        string text2 = "";
        getline(cin, text2);
        cout << endl;
        if (text2 == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        text = text + text2;
        ofstream fout("txt_files/" + filename + ".txt");
        if (fout)
        {
            cout << "Файл редактирован\n";
            fout << text;
            fout.close();
        }
        else {
            cout << "Файл нельзя открыть\n";
            return;
        }

    }
    static void DeleteFile() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для удаления: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (!remove(("txt_files/" + filename + ".txt").c_str()))
        {
            cout << "Файл " << filename << ".txt удалён\n";
        }
        else {
            cout << "Такого файла нет\n";
            return;
        }
    }
};
//Работа с джсон
class JsonHandler {
public:
    static void Json_cycle() {
        int endc = 0;
        string choice = "";
        while (endc == 0) {
            cout << "[Работа с json] Выберите необходимую функцию:\n";
            cout << "1. Информация о текущих json файлах в рабочей директории\n";
            cout << "2. Чтение json файла\n";
            cout << "3. Создание json файла\n";
            cout << "4. Запись данных в json файл\n";
            cout << "5. Удаление json файла\n";
            cout << "6. Выход из работы с json файлами\n";
            cout << "Ваш выбор: ";
            getline(cin, choice);
            cout << endl;
            if ((is_number(choice) == 0) or choice == "")
            {
                cout << "Введено неправильное значение. Повторите.\n\n";
                continue;
            }
            int intchoice = stoi(choice);
            switch (intchoice)
            {
            case 1:
                DirectoryJson();
                break;
            case 2:
                ReadJson();
                break;
            case 3:
                CreateJson();
                break;
            case 4:
                RedactJson();
                break;
            case 5:
                DeleteJson();
                break;
            case 6:
                endc = 1;
                break;
            default:
                cout << "Неизвестная команда";
                break;
            }
            cout << endl;
        }
    }
    static void DirectoryJson() {
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/json_files" })
            std::cout << entry.path().filename() << std::endl;
    }
    static void ReadJson() {
        string filename = "";
        string text = "";
        json data;
        cout << "Введите имя файла для чтения: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        ifstream fin("json_files/" + filename + ".json");
        if (fin) {
            fin >> data;
            cout << "Данные JSON файла:" << endl;
            cout << data.dump(4) << endl;
            fin.close();
        }
        else
        {
            cout << "Файла нет либо его нельзя открыть\n";
            return;
        }
    }
    static void CreateJson() {
        string jsonFile = "data.json";
        json data;
        string pole;
        cout << "Сколько полей у вас будет(макс 10): ";
        getline(cin, pole);
        cout << endl;
        string polename = "";
        if ((is_number(pole) == 0) or pole == "")
        {

            cout << "Введено неправильное значение.\n\n";
            return;
        }
        int intpole = stoi(pole);
        string list[10];
        string listvalues[10];
        for (int i = 0; i < intpole;i++) {
            cout << "Введите имя поля " + to_string(i + 1) + " : ";
            getline(cin, pole);
            list[i] = pole;
            cout << "Введите значение поля " + to_string(i + 1) + " : ";
            getline(cin, pole);
            listvalues[i] = pole;
            cout << endl;
        }
        for (int i = 0; i < intpole;i++) {
            data[list[i]] = listvalues[i];
        }

        string filename = "";
        string text = "";
        cout << "Введите имя создаваемого json файла: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (fileExists("json_files/" + filename + ".json") == 0) {
            ofstream file("json_files/" + filename + ".json");
            file << data.dump(4); // Indented JSON output
            file.close();
        }
        else {
            cout << "Файл с таким именем уже существует\n";
            return;
        }
    }
    static void RedactJson() {
        string filename = "";
        string text = "";
        json data;
        cout << "Введите имя файла для редактирования: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        ifstream fin("json_files/" + filename + ".json");
        if (fin) {
            fin >> data;
            cout << "Данные JSON файла:" << endl;
            cout << data.dump(4) << endl;
            fin.close();
        }
        else
        {
            cout << "Файла нет либо его нельзя открыть\n";
            return;
        }
        json data2;
        data2 = data;
        string param = "";
        cout << "Какой параметр изменить: ";
        getline(cin, param);
        cout << endl;
        if (param == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        string newvalue = "";
        cout << "На какое значение: ";
        getline(cin, newvalue);
        cout << endl;
        if (newvalue == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        data2[param] = newvalue;
        ofstream file("json_files/" + filename + ".json");
        file << data2.dump(4); // Indented JSON output
        file.close();
        cout << "Файл редактирован\n";
    }
    static void DeleteJson() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для удаления: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (!remove(("json_files/" + filename + ".json").c_str()))
        {
            cout << "Файл " << filename << ".json удалён\n";
        }
        else {
            cout << "Такого файла нет\n";
        }
    }
};
//Работа с хмл
class XmlHandler {
public:
    static void Xml_cycle() {
        int endc = 0;
        string choice = "";
        while (endc == 0) {
            cout << "[Работа с xml] Выберите необходимую функцию:\n";
            cout << "1. Информация о текущих xml файлах в рабочей директории\n";
            cout << "2. Чтение xml файла\n";
            cout << "3. Создание xml файла\n";
            cout << "4. Удаление xml файла\n";
            cout << "5. Выход из работы с xml файлами\n";
            cout << "Ваш выбор: ";
            getline(cin, choice);
            cout << endl;
            if ((is_number(choice) == 0) or choice == "")
            {
                cout << "Введено неправильное значение. Повторите.\n\n";
                continue;
            }
            int intchoice = stoi(choice);
            switch (intchoice)
            {
            case 1:
                DirectoryXml();
                break;
            case 2:
                ReadXml();
                break;
            case 3:
                CreateXml();
                break;
            case 4:
                DeleteXml();
                break;
            case 5:
                endc = 1;
                break;
            default:
                cout << "Неизвестная команда";
                break;
            }
            cout << endl;
        }
    }
    //рекурсивный обход дерева
    static void print_xml(const pugi::xml_node& node, int indent = 0) {
        for (int i = 0; i < indent; i++) {
        std::cout << "  "; // Добавление отступа для каждого уровня
    }

    std::cout << "<" << node.name();

    // Вывод атрибутов
    for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
        std::cout << " " << attr.name() << "=\"" << attr.value() << "\"";
    }

    if (node.first_child()) {
        std::cout << ">" << std::endl;
        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
            print_xml(child, indent + 1); // Рекурсивный вызов для дочерних элементов
        }
        for (int i = 0; i < indent; i++) {
            std::cout << "  "; // Добавление отступа перед закрывающим тегом
        }
        std::cout << "</" << node.name() << ">" << std::endl;
    } else {
        std::string value = node.value();
        if (!value.empty()) {
            std::cout << ">" << value << "</" << node.name() << ">" << std::endl;
        } else {
            std::cout << "/>" << std::endl; // Закрытый тег для элемента без содержимого
        }
    }
    }
    static void DirectoryXml() {
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/xml_files" })
            std::cout << entry.path().filename() << std::endl;
    }
    static void ReadXml() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для чтения: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        pugi::xml_document doc;
        if (doc.load_file(("xml_files/" + filename + ".xml").c_str())) {
            std::cout << "XML файл успешно загружен." << std::endl;
            pugi::xml_node root = doc.child("root");

            print_xml(doc.first_child());
        }
        else {
            std::cerr << "Ошибка при загрузке XML файла." << std::endl;
        }


    }
    static void CreateXml() {
        pugi::xml_document doc;
        auto declarationNode = doc.append_child(pugi::node_declaration);
        declarationNode.append_attribute("version") = "1.0";
        declarationNode.append_attribute("encoding") = "ISO-8859-1";
        declarationNode.append_attribute("standalone") = "yes";
        auto root = doc.append_child("root");
        string pole;
        cout << "Сколько полей у вас будет(макс 10): ";
        getline(cin, pole);
        cout << endl;
        string polename = "";
        if ((is_number(pole) == 0) or pole == "")
        {

            cout << "Введено неправильное значение.\n\n";
            return;
        }
        int intpole = stoi(pole);
        string list[10];
        string listvalues[10];
        for (int i = 0; i < intpole;i++) {
            cout << "Введите имя поля " + to_string(i + 1) + " : ";
            getline(cin, pole);
            list[i] = pole;
            cout << "Введите значение поля " + to_string(i + 1) + " : ";
            getline(cin, pole);
            listvalues[i] = pole;
            cout << endl;
        }
        for (int i = 0; i < intpole;i++) {
            pugi::xml_node node = root.append_child((list[i]).c_str());
            node.append_child(pugi::node_pcdata).set_value((listvalues[i]).c_str());
        }



        string filename = "";
        string text = "";
        cout << "Введите имя создаваемого xml файла: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (doc.save_file(("xml_files/" + filename + ".xml").c_str())) {
            cout << "Файл " + filename + ".xml создан";
        }
        else
            cout << "Ошибка в создани файла";
    }
    static void DeleteXml() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для удаления: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (!remove(("xml_files/" + filename + ".xml").c_str()))
        {
            cout << "Файл " << filename << ".xml удалён\n";
        }
        else {
            cout << "Такого файла нет\n";
        }
    }

    static void Write(string filename, string content) {
        cout << "4.2. Запись данных\n";
        pugi::xml_document doc;
        if (doc.load_file(filename.c_str())) {
            pugi::xml_node root = doc.document_element();
            root.text().set(content.c_str());
            doc.save_file(filename.c_str());
            cout << "Введённые данные: " << filename << endl;
        }
        else {
            cout << "Ошибка в создании XML файла: " << filename << endl;
        }
    }
};
//Работа с зип
class ZipHandler {
public:
    static void Zip_cycle() {
        int endc = 0;
        string choice = "";
        while (endc == 0) {
            cout << "[Работа с zip] Выберите необходимую функцию:\n";
            cout << "1. Информация о файлах которые можно заархивировать и об имеющихся архивах\n";
            cout << "2. Разархивирование\n";
            cout << "3. Создание Архива\n";
            cout << "4. Удаление Архива\n";
            cout << "5. Выход из работы с xml файлами\n";
            cout << "Ваш выбор: ";
            getline(cin, choice);
            cout << endl;
            if ((is_number(choice) == 0) or choice == "")
            {
                cout << "Введено неправильное значение. Повторите.\n\n";
                continue;
            }
            int intchoice = stoi(choice);
            switch (intchoice)
            {
            case 1:
                DirectoryZip();
                break;
            case 2:
                UnZip();
                break;
            case 3:
                Zip();
                break;
            case 4:
                DeleteZip();
                break;
            case 5:
                endc = 1;
                break;
            default:
                cout << "Неизвестная команда";
                break;
            }
            cout << endl;
        }
    }
    static void DirectoryZip() {
        cout << "Текстовые файлы:\n";
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/txt_files" })
            std::cout << entry.path().filename() << std::endl;
        cout << "\nJson файлы:\n";
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/json_files" })
            std::cout << entry.path().filename() << std::endl;
        cout << "\nXml файлы:\n";
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/xml_files" })
            std::cout << entry.path().filename() << std::endl;
        cout << "\nZip файлы:\n";
        for (const auto& entry : std::filesystem::directory_iterator{ "C:/Users/cylct/source/repos/Latypov/Latypov/zip_files" })
            std::cout << entry.path().filename() << std::endl;
    }
    static void UnZip() {
        string filename = "";
        cout << "Введите имя файла для разархивирования: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        filename = "zip_files/" + filename + ".zip";
        TCHAR* param = new TCHAR[filename.size() + 1];
        param[filename.size()] = 0;
        //As much as we'd love to, we can't use memcpy() because
        //sizeof(TCHAR)==sizeof(char) may not be true:
        std::copy(filename.begin(), filename.end(), param);

        HZIP hz2 = OpenZip(param, 0);
        ZIPENTRY ze; GetZipItem(hz2, -1, &ze);
        int numitems = ze.index;
        for (int zi = 0; zi < numitems; zi++)
        {
            ZIPENTRY ze; GetZipItem(hz2, zi, &ze);
            UnzipItem(hz2, zi, ze.name);
        }

        CloseZip(hz2);
        cout << "Файл Разархивирован";
    }
    static void Zip() {
        string filename = "";
        cout << "Введите имя архива: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        filename = "zip_files/" + filename + ".zip";
        TCHAR* param = new TCHAR[filename.size() + 1];
        param[filename.size()] = 0;
        std::copy(filename.begin(), filename.end(), param);
        cout << "Какой файл заархивировать (с расширением (.txt, .json, .xml)): ";
        string filetozip = "";
        getline(cin, filetozip);
        cout << endl;
        if (filetozip == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        string extension = "";
        string folder = "";
        int extensionpoint = 0;
        for (int i = 0; i < filetozip.size();i++) {
            if (extensionpoint == 0) {
                if (filetozip[i] == '.') {
                    extensionpoint = 1;
                }
            }
            else
                extension += filetozip[i];
        }
        if (extension == "txt") folder = "txt_files";
        if (extension == "json") folder = "json_files";
        if (extension == "xml") folder = "xml_files";
        TCHAR* param2 = new TCHAR[filetozip.size() + 1];
        param2[filetozip.size()] = 0;
        std::copy(filetozip.begin(), filetozip.end(), param2);
        filetozip = folder + "/" + filetozip;
        TCHAR* param3 = new TCHAR[filetozip.size() + 1];
        param3[filetozip.size()] = 0;
        std::copy(filetozip.begin(), filetozip.end(), param3);
        HZIP hz = CreateZip(param, 0);
        ZipAdd(hz, param2, param3);
        CloseZip(hz);
        cout << "Файл архивирован";
    }
    static void DeleteZip() {
        string filename = "";
        string text = "";
        cout << "Введите имя файла для удаления: ";
        getline(cin, filename);
        cout << endl;
        if (filename == "")
        {
            cout << "Введено неправильное значение.\n\n";
            return;
        }
        if (!remove(("zip_files/" + filename + ".zip").c_str()))
        {
            cout << "Файл " << filename << ".zip удалён\n";
        }
        else {
            cout << "Такого файла нет\n";
        }
    }
};
//Работа с потоками

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);

    std::string hashStr;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        hashStr += hex;
    }

    return hashStr;
}

// Функция для проверки, соответствует ли хеш заданному значению
bool isHashMatching(const std::string& input, const std::string& targetHash) {
    std::string hash = sha256(input);
    return hash == targetHash;
}

// Функция для перебора паролей в заданном диапазоне
void bruteForcePasswords(const std::string& targetHash, char start, char end, int threadNum, int numThreads) {
    std::string password(5, 'a');

    for (password[0] = start; password[0] <= end; password[0]++) {
        for (password[1] = 'a'; password[1] <= 'z'; password[1]++) {
            for (password[2] = 'a'; password[2] <= 'z'; password[2]++) {
                for (password[3] = 'a'; password[3] <= 'z'; password[3]++) {
                    for (password[4] = 'a'; password[4] <= 'z'; password[4]++) {
                        if (isHashMatching(password, targetHash)) {
                            std::cout << "Поток " << threadNum << " Пароль найден: " << password << std::endl;
                            return;
                        }
                    }
                }
            }
        }
    }
}

void PasswdStream() {
    std::string targetHash = "1115dd800feaacefdf481f1f9070374a2a81e27880f187396db67958b207cbad";
    string choice;
    // Многопоточный режим
    cout << "Введите число потоков(1-4): ";
    getline(cin, choice);
    cout << endl;
    if ((is_number(choice) <= 0) or choice == "" or (is_number(choice) > 4))
    {
        cout << "Введено неправильное значение. Повторите.\n\n";
        return;
    }

    int numThreads = stoi(choice); // Количество потоков (можно изменить)
    std::vector<std::thread> threads;
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        char start = 'a' + (i * ('z' - 'a' + 1) / numThreads);
        char end = 'a' + ((i + 1) * ('z' - 'a' + 1) / numThreads) - 1;
        threads.emplace_back(bruteForcePasswords, targetHash, start, end, i, numThreads);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    std::cout << "Времени потребовалось: " << duration << " сек" << std::endl;
}
void ConsoleWhile() {
    int endc = 0;
    string choice = "";
    while (endc == 0) {
        cout << "[Меню] Выберите необходимую функцию:\n";
        cout << "1. Информация о дисках\n";
        cout << "2. Работа с файлами\n";
        cout << "3. Работа с json\n";
        cout << "4. Работа с xml\n";
        cout << "5. Работа с zip архивами\n";
        cout << "6. Работа с потоками\n";
        cout << "7. Завершить работу, капитан?\n";
        cout << "Ваш выбор: ";
        getline(cin, choice);
        cout << endl;
        if ((is_number(choice) == 0) or choice == "")
        {
            cout << "Введено неправильное значение. Повторите.\n\n";
            continue;
        }

        int intchoice = stoi(choice);
        switch (intchoice)
        {
        case 1:
            GetInfo();
            break;
        case 2:
            FileHandler::File_cycle();
            break;
        case 3:
            JsonHandler::Json_cycle();
            break;
        case 4:
            XmlHandler::Xml_cycle();
            break;
        case 5:
            ZipHandler::Zip_cycle();
            break;
        case 6:
            PasswdStream();
            break;
        case 7:
            endc = 1;
            break;
        default:
            cout << "Неизвестная команда ";
            break;
        }
        cout << endl << endl;

    }
}
int main()
{
    system("chcp 1251");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");
    ConsoleWhile();
    return 0;
}
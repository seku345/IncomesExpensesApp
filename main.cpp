#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "map"
#include "ctime"
#include "limits"
#include "windows.h"
#include "regex"
#include "include/rapidcsv.h"

struct Transaction
{
    float value = 0.0;
    std::string description;
    std::string category; //TODO ENUM WITH CATEGORIES?
    std::string transfer = "0";
    std::string datetime;
};

struct User
{
    std::string login;
    std::string password;
    std::string name;
    float balance = 0.0;
    std::string path_to_data;
    std::vector<Transaction> data;

};

void clear_screen(char fill = ' ')
{
    COORD tl = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

void load_db(std::string const& path, std::vector<Transaction>& data)
{
    // checking if the file exists
    std::fstream file(path);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    rapidcsv::Document doc(path,
                           rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(';'));
    size_t transactions_count = doc.GetRowCount();
    if (transactions_count == 0)
    {
        return;
    }
    for (size_t i = 0; i < transactions_count; ++i)
    {
        float value = std::stof(doc.GetColumn<std::string>("value")[i]);
        std::string description = doc.GetColumn<std::string>("description")[i];
        std::string category = doc.GetColumn<std::string>("category")[i];
        std::string transfer = doc.GetColumn<std::string>("transfer")[i];
        std::string datetime = doc.GetColumn<std::string>("datetime")[i];

        Transaction transaction = {value, description, category, transfer, datetime};
        data.push_back(transaction);
    }
}

void save_db(std::string const& path, std::vector<Transaction> const& data)
{
    // creating empty file if it's not exists
    std::ofstream file(path);
    if (file.is_open()) file.close();
    else std::cerr << "An error was occurred!";

    rapidcsv::Document doc(path,
                           rapidcsv::LabelParams(-1, -1),
                           rapidcsv::SeparatorParams(';'));
    // preparing to saving
    doc.Clear();
    std::vector<std::string> label_line = {"value",
                                           "description",
                                           "category",
                                           "transfer",
                                           "datetime"};
    int row_index = 0;
    doc.SetRow<std::string>(row_index, label_line);

    for (Transaction const& transaction : data)
    {
        row_index++;
        std::vector<std::string> line = {std::to_string(transaction.value),
                                         transaction.description,
                                         transaction.category,
                                         transaction.transfer,
                                         transaction.datetime};
        doc.SetRow<std::string>(row_index, line);
    }
    doc.Save(path);
}

void load(std::string const& path, std::map<std::string, User>& users)
{
    // checking if the file exists
    std::fstream file(path);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    rapidcsv::Document doc(path,
                           rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(';'));

    size_t users_count = doc.GetRowCount();
    if (users_count == 0)
    {
        return;
    }
    for (size_t i = 0; i < users_count; ++i)
    {
        std::string login = doc.GetColumn<std::string>("login")[i];
        std::string password = doc.GetColumn<std::string>("password")[i];
        std::string name = doc.GetColumn<std::string>("name")[i];
        float balance = doc.GetColumn<float>("balance")[i];

        std::string path_to_data = doc.GetColumn<std::string>("path_to_data")[i];

        std::vector<Transaction> data;
        load_db(path_to_data, data);

        User user = {login, password, name, balance, path_to_data, data};
        users.emplace(login, user);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-extensions"
void save(std::string const& path, std::map<std::string, User> const& users)
{
    // creating empty file if it's not exists
    std::ofstream file(path);
    if (file.is_open()) file.close();
    else std::cerr << "An error was occurred!";

    rapidcsv::Document doc(path,
                           rapidcsv::LabelParams(-1, -1),
                           rapidcsv::SeparatorParams(';'));

    // preparing to saving
    doc.Clear();
    std::vector<std::string> label_line = {"login",
                                           "password",
                                           "name",
                                           "balance",
                                           "path_to_data"};
    int row_index = 0;
    doc.SetRow<std::string>(row_index, label_line);

    for (auto const& [login, user]: users)
    {
        row_index++;
        std::vector<std::string> line = {user.login,
                                         user.password,
                                         user.name,
                                         std::to_string(user.balance),
                                         user.path_to_data};
        doc.SetRow<std::string>(row_index, line);
        save_db(user.path_to_data, user.data);
    }
    doc.Save(path);
}
#pragma clang diagnostic pop

void Login(std::map<std::string, User>& users, std::string& active_user);
void Registration(std::map<std::string, User>& users, std::string& active_user);

void Login(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    std::string login, password;
    std::cout << "Enter your login or R letter if you want to register: ";
    std::cin >> login;
    if (login == "R" or login == "r")
    {
        Registration(users, active_user);
        return;
    }
    else if (users.contains(login))
    {
        bool is_password_correct = false;
        do {
            std::cout << "Enter your password: ";
            std::cin >> password;
            if (users[login].password == password)
            {
                is_password_correct = true;
                std::cout << "Welcome back!\n";
                active_user = users[login].login;
            }
            else
            {
                std::cout << "Incorrect password! Try again";
            }
        } while (not is_password_correct);
    }
    else
    {
        std::cout << "This login isn't in our system. Maybe you made mistake, try again.\n";
        Login(users, active_user);
        return;
    }
}

void Registration(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    std::string login, password;
    char name[32];
    std::cout << "Make up a login or type L letter if you want to login: ";
    std::cin >> login;
    if (login == "L" or login == "l")
    {
        Login(users, active_user);
        return;
    }
    else if (not users.contains(login))
    {
        bool is_password_correct = false;
        std::regex password_pattern("^(?=.*[A-Z])(?=.*\\d)(?=.*\\W).{8,}$");
        do {
            std::cout << "Make up a password: ";
            std::cin >> password;
            if (std::regex_match(password, password_pattern))
            {
                is_password_correct = true;
                std::cout << "Write your name: ";
                std::cin.getline(name, 32);
                float balance = 0.0;
                std::string path_to_data = "../transactions/" + login + ".csv";

                User new_user = {login, password, name, balance, path_to_data};
                users.emplace(new_user.login, new_user);
                active_user = new_user.login;
            }
            else
            {
                std::cout << "It's too weak password. It must contain at least 1 capital letter, 1 digit, 1 special character and be at least 8 in length.\n";
            }
        } while (not is_password_correct);
    }
    else
    {
        std::cout << "This login is already registered. Try another or login.\n";
        Registration(users, active_user);
        return;
    }
}

void authorisation(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    std::cout << "Hello! Welcome to InExApp!\n" <<
                "Are you already have an account in our system?\n" <<
                "Type Y if yes or N if no and you want to create a new account: ";
    std::string choice;
    std::cin >> choice;
    if (choice == "Y" or choice == "y")
    {
        Login(users, active_user);
        return;
    }
    else if (choice == "N" or choice == "n")
    {
        Registration(users, active_user);
        return;
    }
    else
    {
        std::cout << "Invalid input! Try again.\n";
        authorisation(users, active_user);
        return;
    }
}

void main_menu(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    std::cout
}

int main()
{
    std::map<std::string, User> users;
    load("../users.csv", users);

    std::string active_user = users["Admin1337"].login;

    authorisation(users, active_user);

    main_menu(users, active_user);

    std::cout << active_user;

    save("../users.csv", users);
}
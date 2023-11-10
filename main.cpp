#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "map"
#include "chrono"
#include "iomanip"
#include "limits"
#include "windows.h"
#include "regex"
#include "include/rapidcsv.h"

struct Transaction
{
    float value = 0.0;
    std::string description;
    std::string category;
    std::string datetime;
};

struct User
{
    std::string login;
    std::string password; //TODO PASSWORD ENCRYPTION
    std::string name;
    float balance = 0.0;
    std::string path_to_data;
    std::vector<Transaction> data;

};

void switch_color(std::string const& color = "white")
{
    int color_code = 15;
    if (color == "red") color_code = 12;
    else if (color == "yellow") color_code = 14;
    else if (color == "green") color_code = 10;
    else if (color == "blue") color_code = 9;
    else if (color == "black") color_code = 16;
    else if (color == "white") color_code = 15;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color_code);
}

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

std::string get_current_datetime()
{
    auto now = std::chrono::system_clock::now();
    time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&current_time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y.%m.%d %H:%M:%S");
    std::string datetime = ss.str();
    return datetime;
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
        std::string datetime = doc.GetColumn<std::string>("datetime")[i];

        Transaction transaction = {value, description, category, datetime};
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
                                           "datetime"};
    int row_index = 0;
    doc.SetRow<std::string>(row_index, label_line);

    for (Transaction const& transaction : data)
    {
        row_index++;
        std::vector<std::string> line = {std::to_string(transaction.value),
                                         transaction.description,
                                         transaction.category,
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

void Login(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account);
void Registration(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account);

void Login(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account)
{
    clear_screen();

    std::string login, password;
    std::cout << "Enter your login or R letter if you want to register\nor 0 to exit: ";
    std::cin >> login;
    if (login == "0")
    {
        is_working = false;
        return;
    }
    else if (login == "R" or login == "r")
    {
        Registration(users, active_user, is_working, is_in_account);
        return;
    }
    else if (users.contains(login))
    {
        bool is_password_correct;
        do {
            is_password_correct = false;
            std::cout << "Enter your password: ";
            std::cin >> password;
            if (users[login].password == password)
            {
                is_password_correct = true;
                std::cout << "Welcome back!\n";
                active_user = users[login].login;
                is_in_account = true;
            }
            else
            {
                std::cout << "Incorrect password! Try again.";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_password_correct);
    }
    else
    {
        std::cout << "This login isn't in our system. Maybe you made mistake, try again.\n";
        Sleep(3000);
        Login(users, active_user, is_working, is_in_account);
        return;
    }
}

void Registration(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account)
{
    clear_screen();

    std::string login, password;
    char name[32];
    std::cout << "Make up a login or type L letter if you want to login\nor 0 to exit: ";
    std::cin >> login;
    if (login == "0")
    {
        is_working = false;
        return;
    }
    if (login == "L" or login == "l")
    {
        Login(users, active_user, is_working, is_in_account);
        return;
    }
    else if (not users.contains(login) and (login.length() >= 4))
    {
        bool is_password_correct;
        std::regex password_pattern("^(?=.*[A-Z])(?=.*\\d)(?=.*\\W).{8,}$");
        do {
            is_password_correct = false;
            std::cout << "Make up a password: ";
            std::cin >> password;
            if (std::regex_match(password, password_pattern))
            {
                is_password_correct = true;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Write your name: ";
                std::cin.getline(name, 32);
                float balance = 0.0;
                std::string path_to_data = "../transactions/" + login + ".csv";

                User new_user = {login, password, name, balance, path_to_data};
                users.emplace(new_user.login, new_user);
                active_user = new_user.login;
                is_in_account = true;
            }
            else
            {
                std::cout << "It's too weak password. It must contain at least 1 capital letter, 1 digit,\n1 special character and be at least 8 in length.\n";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_password_correct);
    }
    else if (users.contains(login) and (login.length() >= 4))
    {
        std::cout << "This login is already registered. Try another or login.\n";
        Sleep(3000);
        Registration(users, active_user, is_working, is_in_account);
        return;
    }
    else
    {
        std::cout << "This login is too short. It must to be at least 4 characters long.\nTry again.\n";
        Sleep(3000);
        Registration(users, active_user, is_working, is_in_account);
        return;
    }
}

void authorisation(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account)
{
    clear_screen();

    std::cout << "Hello! Welcome to InExApp!\n"
              << "Are you already have an account in our system?\n"
              << "Type Y if yes or N if no and you want to create a new account\nor 0 if you want to exit: ";
    std::string choice;
    std::cin >> choice;
    if (choice == "0")
    {
        is_working = false;
        return;
    }
    else if (choice == "Y" or choice == "y")
    {
        Login(users, active_user, is_working, is_in_account);
        return;
    }
    else if (choice == "N" or choice == "n")
    {
        Registration(users, active_user, is_working, is_in_account);
        return;
    }
    else
    {
        std::cout << "Invalid input! Try again.\n";
        authorisation(users, active_user, is_working, is_in_account);
        return;
    }
}

bool is_in(std::string const& x, std::vector<std::string> const& a)
{
    for (std::string const& s : a)
    {
        if (s == x)
        {
            return true;
        }
    }
    return false;
}

void new_transaction(std::map<std::string, User>& users, std::string& active_user) //TODO REWRITING OF THIS FUNCTION!!! (I HATE IT.)
{
    clear_screen();

    std::map<int, std::string> categories = {{1, "Transfer to another person"},
                                             {2, "Food & Beverages"},
                                             {3, "Entertainment"},
                                             {4, "Transportation"},
                                             {5, "Health & Medicals"},
                                             {6, "Housing & Utilities"},
                                             {7, "Clothing & Footwear"},
                                             {8, "Education"},
                                             {9, "Electronics & Gadgets"},
                                             {10, "Travel & Vacation"},
                                             {11, "Gifts & Donation"},
                                             {12, "Salary"},
                                             {13, "Investments"},
                                             {14, "Passive Income"},
                                             {15, "Sales & Business"},
                                             {16, "Other transactions"}};

    std::cout << "What kind of transaction is it?\n";
    for (auto const& [num, ctg] : categories)
    {
        if (num < 10) std::cout << ' ';
        std::cout << num << ". " << ctg << '\n';
    }
    std::cout << "\n\nOr type 0 to return to the main menu.\n";

    std::string choice;
    std::cin >> choice;
    std::vector<std::string> const variants_exp = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};
    std::vector<std::string> const variants_inc = {"12", "13", "14", "15"};
    clear_screen();
    if (choice == "0")
    {
        return;
    }
    else if (choice == "1") // transfer
    {
        bool is_login_correct;
        do {
            is_login_correct = true;
            std::cout << "Enter the login of the person you want to transfer funds to\nOr type 0 to get back: ";
            std::string login;
            std::cin >> login;
            if (login == "0")
            {
                new_transaction(users, active_user);
                return;
            }
            else if (login == active_user)
            {
                std::cout << "You can't transfer funds to yourself! Try another category to do this.\n";
                Sleep(3000);
                new_transaction(users, active_user);
                return;
            }
            else if (users.contains(login))
            {
                bool is_value_correct;
                do {
                    is_value_correct = true;
                    std::cout << "Enter how much money do you want to transfer or 0 to get back: ";
                    std::string value_str;
                    std::cin >> value_str;
                    std::regex value_pattern("^[0-9]+(\\.[0-9]{0,2})?$");
                    if (value_str == "0")
                    {
                        new_transaction(users, active_user);
                        return;
                    }
                    else if (std::regex_match(value_str, value_pattern) and (users[active_user].balance >= std::stof(value_str)))
                    {
                        float const value = std::stof(value_str);
                        std::string const description_to = "Transfer to " + login;
                        std::string const description_from = "Transfer from " + active_user;
                        std::string const category = "Transfer";
                        std::string const datetime = get_current_datetime();
                        Transaction transaction_from = {value, description_from, category, datetime};
                        Transaction transaction_to = {-value, description_to, category, datetime};

                        users[active_user].data.push_back(transaction_to);
                        users[login].data.push_back(transaction_from);

                        users[login].balance += value;
                        users[active_user].balance -= value;
                    }
                    else if (std::regex_match(value_str, value_pattern) and (users[active_user].balance < std::stof(value_str)))
                    {
                        is_value_correct = false;
                        std::cout << "You don't have enough money to do this transaction.\nTry another value.\n";
                        Sleep(3000);
                        clear_screen();
                    }
                    else
                    {
                        is_value_correct = false;
                        std::cout << "It's uncorrected value. Try again.\n";
                        Sleep(3000);
                        clear_screen();
                    }
                } while (not is_value_correct);
            }
            else
            {
                is_login_correct = false;
                std::cout << "There is no user with this login. Try again.\n";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_login_correct);
    }
    else if (is_in(choice, variants_exp)) // expenses
    {
        std::cout << "Write the short description of the transaction: ";
        char description[32];
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.getline(description, 32);

        bool is_value_correct;
        do {
            is_value_correct = true;
            std::cout << "Enter the value of the expense or 0 to get back: ";
            std::string value_str;
            std::cin >> value_str;
            std::regex value_pattern("^[0-9]+(\\.[0-9]{0,2})?$");
            if (value_str == "0")
            {
                new_transaction(users, active_user);
                return;
            }
            else if (std::regex_match(value_str, value_pattern) and (users[active_user].balance >= std::stof(value_str)))
            {
                float value = std::stof(value_str);
                std::string category = categories[std::stoi(choice)];
                std::string datetime = get_current_datetime();
                Transaction transaction = {-value, description, category, datetime};
                users[active_user].data.push_back(transaction);
                users[active_user].balance -= value;
            }
            else if (std::regex_match(value_str, value_pattern) and (users[active_user].balance < std::stof(value_str)))
            {
                std::cout << "You don't have enough money to make this transaction!\nReturning to the main menu...";
                Sleep(3000);
                return;

            }
            else
            {
                is_value_correct = false;
                std::cout << "Invalid input! Try again.\n";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_value_correct);

    }
    else if (is_in(choice, variants_inc)) // incomes
    {
        std::cout << "Write the short description of the transaction: ";
        char description[32];
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.getline(description, 32);

        bool is_value_correct;
        do {
            is_value_correct = true;
            std::cout << "Enter the value of the income or 0 to get back: ";
            std::string value_str;
            std::cin >> value_str;
            std::regex value_pattern("^[0-9]+(\\.[0-9]{0,2})?$");
            if (value_str == "0")
            {
                new_transaction(users, active_user);
                return;
            }
            else if (std::regex_match(value_str, value_pattern))
            {
                float value = std::stof(value_str);
                std::string category = categories[std::stoi(choice)];
                std::string datetime = get_current_datetime();
                Transaction transaction = {value, description, category, datetime};
                users[active_user].data.push_back(transaction);
                users[active_user].balance += value;
            }
            else
            {
                is_value_correct = false;
                std::cout << "Invalid input! Try again.\n";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_value_correct);
    }
    else if (choice == "16") // other
    {
        std::cout << "Write the short description of the transaction: ";
        char description[32];
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.getline(description, 32);

        bool is_value_correct;
        do {
            is_value_correct = true;
            std::cout << "Enter the value of the transaction (with - if it's expense or without if no)\n or 0 to get back: ";
            std::string value_str;
            std::cin >> value_str;
            std::regex value_pattern("^-{0,1}[0-9]+(\\.[0-9]{0,2})?$");
            if (value_str == "0")
            {
                new_transaction(users, active_user);
                return;
            }
            else if (std::regex_match(value_str, value_pattern) and ((users[active_user].balance >= std::stof(value_str)) and (std::stof(value_str) < 0) or (std::stof(value_str) > 0)))
            {
                float value = std::stof(value_str);
                std::string category = categories[std::stoi(choice)];
                std::string datetime = get_current_datetime();
                Transaction transaction = {value, description, category, datetime};
                users[active_user].data.push_back(transaction);
                users[active_user].balance += value;
            }
            else if (std::regex_match(value_str, value_pattern) and ((users[active_user].balance < std::stof(value_str)) and (std::stof(value_str) < 0)))
            {
                std::cout << "You don't have enough money to make this transaction!\nReturning to the main menu...";
                Sleep(3000);
                return;
            }
            else
            {
                is_value_correct = false;
                std::cout << "Invalid input! Try again.\n";
                Sleep(3000);
                clear_screen();
            }
        } while (not is_value_correct);
    }
    else
    {
        std::cout << "Invalid input! Try again.\n";
        Sleep(3000);
        new_transaction(users, active_user);
        return;
    }
}

int max_length(std::vector<std::string> const& vec)
{
    int max = 0;
    for (std::string const& s : vec)
    {
        max = std::max(max, int(s.length()));
    }
    return max;
}

std::string multi_string(std::string const& s, int const& n)
{
    std::string r;
    for (int i = 0; i < n; ++i)
    {
        r += s;
    }
    return r;
}

void transactions_menu(std::map<std::string, User>& users, std::string& active_user)
{
    std::cout << "Choose what you want to do:\n"
              << "Bla-bla-bla\n"
              << "\nOr type 0 to get back to the main menu.\n";
    std::string choice;
    std::cin >> choice;
    if (choice == "0")
    {
        return;
    }
    else
    {

    }
}

void get_transactions(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    // getting vectors-columns
    size_t const data_size = users[active_user].data.size();
    std::vector<std::string> value_column(data_size+1);
    value_column[0] = "Value";
    std::vector<std::string> description_column(data_size+1);
    description_column[0] = "Description";
    std::vector<std::string> category_column(data_size+1);
    category_column[0] = "Category";
    std::vector<std::string> datetime_column(data_size+1);
    datetime_column[0] = "Datetime";
    for (size_t i = 0; i < data_size; ++i)
    {
        std::ostringstream stream;
        float value = users[active_user].data[i].value;
        stream << std::fixed << std::setprecision(2) << value;
        std::string value_str = stream.str();
        value_column[i+1] = value_str;
        description_column[i+1] = users[active_user].data[i].description;
        category_column[i+1] = users[active_user].data[i].category;
        datetime_column[i+1] = users[active_user].data[i].datetime;
    }

    // getting max lengths
    int max_value_length = max_length(value_column);
    int max_description_length = max_length(description_column);
    int max_category_length = max_length(category_column);
    int max_datetime_length = max_length(datetime_column);

    for (int i = 0; i < data_size+1; ++i)
    {
        std::cout << value_column[i] + multi_string(" ", max_value_length - int(value_column[i].length()) + 2)
                  << description_column[i] + multi_string(" ", max_description_length - int(description_column[i].length()) + 2)
                  << category_column[i] + multi_string(" ", max_category_length - int(category_column[i].length()) + 2)
                  << datetime_column[i] + multi_string(" ", max_datetime_length - int(datetime_column[i].length()) + 2)
                  << '\n';
    }
    std::cout << '\n';
    transactions_menu(users, active_user);
}

void budgets(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();
}

//void calc_and_draw_diagram(std::map<std::string, float> const& total_value_by_category)
//{
//    std::map<std::string, std::string> category_colors = {{1, "Transfer to another person"},
//                                                          {2, "Food & Beverages"},
//                                                          {3, "Entertainment"},
//                                                          {4, "Transportation"},
//                                                          {5, "Health & Medicals"},
//                                                          {6, "Housing & Utilities"},
//                                                          {7, "Clothing & Footwear"},
//                                                          {8, "Education"},
//                                                          {9, "Electronics & Gadgets"},
//                                                          {10, "Travel & Vacation"},
//                                                          {11, "Gifts & Donation"},
//                                                          {12, "Salary"},
//                                                          {13, "Investments"},
//                                                          {14, "Passive Income"},
//                                                          {15, "Sales & Business"},
//                                                          {16, "Other transactions"}};
//    for (auto const& [category, total_value] : total_value_by_category)
//    {
//
//    }
//}

bool is_date_within_last_week(std::string const& datetime)
{
    time_t cur_time = std::time(nullptr);
    tm* cur_tm = std::localtime(&cur_time);

    tm last_week_start_tm = *cur_tm;
    last_week_start_tm.tm_mday -= 7;

}

bool is_date_within_last_month(std::string const& datetime)
{

}

bool is_date_within_last_year(std::string const& datetime)
{

}

void view_statistics(std::map<std::string, User>& users, std::string& active_user)
{
    clear_screen();

    std::cout << "Choose the period you want to see:\n"
              << "1. Today\n"
              << "2. Last week\n"
              << "3. Last month\n"
              << "4. Last year\n"
              << "5. All time\n"
              << "\nOr type 0 to get back to the main menu.\n";
    std::string choice;
    std::cin >> choice;
    std::vector<Transaction> transactions;
    if (choice == "0")
    {
        return;
    }
    else if (choice == "1")
    {
        for (Transaction const& transaction : users[active_user].data)
        {
            std::string const date = transaction.datetime.substr(0, transaction.datetime.find(' '));
            std::string const cur_date = get_current_datetime().substr(0, get_current_datetime().find(' '));

            if (date == cur_date)
            {
                transactions.push_back(transaction);
            }
        }
    }
    else if (choice == "2")
    {

    }
    else if (choice == "3")
    {

    }
    else if (choice == "4")
    {

    }
    else if (choice == "5")
    {

    }
    else
    {
        std::cout << "Invalid input! Try again.\n";
        Sleep(3000);
        view_statistics(users, active_user);
        return;
    }
    std::map<std::string, int> total_income;
    std::map<std::string, int> total_expense;
    
}

void main_menu(std::map<std::string, User>& users, std::string& active_user, bool& is_working, bool& is_in_account)
{
    clear_screen();

    std::cout << "Hello, ";
    switch_color("yellow");
    std::cout << users[active_user].name << '\n';
    switch_color("white");
    std::cout << "Here are some information about your account:\n"
              << "Login: " << users[active_user].login << '\n'
              << "Password: " << users[active_user].password << '\n'
              << "Name: " << users[active_user].name << '\n'
              << "Balance: " << users[active_user].balance << '\n';
    std::string action;
    bool is_action_correct;
    do {
        is_action_correct = true;
        std::cout << "\nWhat you want to do?\n"
                  << "1. Make a new transaction\n"
                  << "2. View transaction history\n"
                  << "3. Set the budget\n"
                  << "4. View statistics\n"
                  << "5. Sign out\n";
        std::cin >> action;
        if (action == "1")
        {
            new_transaction(users, active_user);
        }
        else if (action == "2")
        {
            get_transactions(users, active_user);
        }
        else if (action == "3")
        {
            budgets(users, active_user);
        }
        else if (action == "4")
        {
            view_statistics(users, active_user);
        }
        else if (action == "5")
        {
            std::cout << "Saving your data and exiting...\n";
            Sleep(3000);
            is_in_account = false;
            return;
        }
        else
        {
            is_action_correct = false;
            std::cout << "Invalid input! Try again.\n";
            Sleep(3000);
            clear_screen();
        }
    } while (not is_action_correct);
}

int main()
{
    std::map<std::string, User> users;
    load("../users.csv", users);

    std::string active_user;

    bool is_working = true;
    bool is_in_account = false;

    while (is_working)
    {
        if (not is_in_account)
        {
            authorisation(users, active_user, is_working, is_in_account);
        }
        else
        {
            main_menu(users, active_user, is_working, is_in_account);
        }
        save("../users.csv", users);
    }
}
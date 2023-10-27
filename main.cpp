#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "array"
#include "map"
#include "ctime"
#include "limits"
#include "windows.h"
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

int main()
{
    std::map<std::string, User> users;
    load("../users.csv", users);

    save("../users.csv", users);
//    save_db("test.csv", users["Admin1337"].data);
//    std::cout << users.size() << ' ' << users["Admin1337"].balance << ' ' << users["Admin1337"].data.size() << ' ' << users["Admin1337"].data[3].datetime << '\n';
//    std::cout << users["musicMaestro"].data.size();
}
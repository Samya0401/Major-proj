#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class Book {
    string id, title, author;
    int quantity = 0;
public:
    Book() = default;
    Book(string id, string title, string author, int quantity) : id(move(id)), title(move(title)), author(move(author)), quantity(quantity) {}
    const string& getId() const { return id; }
    const string& getTitle() const { return title; }
    const string& getAuthor() const { return author; }
    int getQuantity() const { return quantity; }
    bool available() const { return quantity > 0; }
    void setDetails(const string& titleValue, const string& authorValue, int quantityValue) { title = titleValue; author = authorValue; quantity = quantityValue; }
    void issue() { --quantity; }
    void giveBack() { ++quantity; }
    string serialize() const { return id + "|" + title + "|" + author + "|" + to_string(quantity); }
    static bool deserialize(const string& line, Book& book) {
        stringstream ss(line); string id, title, author, quantity;
        if (!getline(ss, id, '|') || !getline(ss, title, '|') || !getline(ss, author, '|') || !getline(ss, quantity)) return false;
        try { book = Book(id, title, author, stoi(quantity)); return true; } catch (...) { return false; }
    }
};

class Person {
protected:
    string id, name;
public:
    Person() = default;
    Person(string id, string name) : id(move(id)), name(move(name)) {}
    virtual ~Person() = default;
    const string& getId() const { return id; }
    const string& getName() const { return name; }
    virtual string role() const = 0;
};

class User : public Person {
    vector<string> issuedBookIds;
public:
    User() = default;
    User(string id, string name) : Person(move(id), move(name)) {}
    string role() const override { return "User"; }
    const vector<string>& issued() const { return issuedBookIds; }
    bool hasBook(const string& bookId) const { return find(issuedBookIds.begin(), issuedBookIds.end(), bookId) != issuedBookIds.end(); }
    void issue(const string& bookId) { issuedBookIds.push_back(bookId); }
    bool returnBook(const string& bookId) { auto it = find(issuedBookIds.begin(), issuedBookIds.end(), bookId); if (it == issuedBookIds.end()) return false; issuedBookIds.erase(it); return true; }
    string serialize() const { string out = id + "|" + name + "|"; for (size_t i = 0; i < issuedBookIds.size(); ++i) out += issuedBookIds[i] + (i + 1 == issuedBookIds.size() ? "" : ","); return out; }
    static bool deserialize(const string& line, User& user) { stringstream ss(line); string id, name, books; if (!getline(ss,id,'|') || !getline(ss,name,'|')) return false; getline(ss,books); user = User(id,name); string item; stringstream list(books); while (getline(list,item,',')) if (!item.empty()) user.issue(item); return true; }
};

class Library {
    vector<Book> books; vector<User> users;
    const string booksFile = "books.txt", usersFile = "users.txt";
    Book* findBook(const string& id) { for (auto& b : books) if (b.getId() == id) return &b; return nullptr; }
    User* findUser(const string& id) { for (auto& u : users) if (u.getId() == id) return &u; return nullptr; }
    static string readLine(const string& prompt) { cout << prompt; string s; getline(cin >> ws, s); return s; }
    static int readPositive(const string& prompt) { int n; cout << prompt; if (!(cin >> n) || n < 0) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return -1; } return n; }
public:
    Library() { load(); }
    void load() { ifstream bf(booksFile), uf(usersFile); string line; Book b; User u; while (getline(bf,line)) if (Book::deserialize(line,b)) books.push_back(b); while (getline(uf,line)) if (User::deserialize(line,u)) users.push_back(u); }
    void save() const { ofstream bf(booksFile), uf(usersFile); for (const auto& b : books) bf << b.serialize() << '\n'; for (const auto& u : users) uf << u.serialize() << '\n'; }
    void addBook() { string id = readLine("Book ID: "); if (id.empty() || findBook(id)) { cout << "Invalid or duplicate Book ID.\n"; return; } string title = readLine("Book name: "), author = readLine("Author name: "); int qty = readPositive("Quantity: "); if (title.empty() || author.empty() || qty < 0) { cout << "Invalid book details.\n"; return; } books.emplace_back(id,title,author,qty); save(); cout << "Book added.\n"; }
    void removeBook() { string id = readLine("Book ID to remove: "); auto it = find_if(books.begin(),books.end(),[&](const Book& b){return b.getId()==id;}); if (it == books.end()) { cout << "Book ID not found.\n"; return; } for (const auto& u: users) if (u.hasBook(id)) { cout << "Cannot remove: this book is currently issued.\n"; return; } books.erase(it); save(); cout << "Book removed.\n"; }
    void updateBook() { string id = readLine("Book ID to update: "); Book* b = findBook(id); if (!b) { cout << "Book ID not found.\n"; return; } string title = readLine("New book name: "), author = readLine("New author name: "); int qty = readPositive("New quantity: "); if(title.empty() || author.empty() || qty < 0) { cout << "Invalid details.\n"; return; } b->setDetails(title,author,qty); save(); cout << "Book updated.\n"; }
    void viewBooks() const { if (books.empty()) { cout << "No books available.\n"; return; } cout << left << setw(12) << "ID" << setw(28) << "Title" << setw(22) << "Author" << setw(10) << "Quantity" << "Status\n"; for(const auto& b:books) cout << setw(12)<<b.getId()<<setw(28)<<b.getTitle().substr(0,27)<<setw(22)<<b.getAuthor().substr(0,21)<<setw(10)<<b.getQuantity()<<(b.available()?"Available":"Unavailable")<<'\n'; }
    void addUser() { string id=readLine("User ID: "); if(id.empty() || findUser(id)) { cout << "Invalid or duplicate User ID.\n"; return; } string name=readLine("User name: "); if(name.empty()) { cout << "Name cannot be empty.\n"; return; } users.emplace_back(id,name); save(); cout << "User registered.\n"; }
    void searchBooks() const { string term=readLine("Search title or author: "); bool any=false; for(const auto& b:books) { string hay=b.getTitle()+" "+b.getAuthor(); transform(hay.begin(),hay.end(),hay.begin(),::tolower); string needle=term; transform(needle.begin(),needle.end(),needle.begin(),::tolower); if(hay.find(needle)!=string::npos) { cout<<b.getId()<<" | "<<b.getTitle()<<" by "<<b.getAuthor()<<" | "<<(b.available()?"Available":"Unavailable")<<'\n'; any=true; } } if(!any) cout<<"No matching books.\n"; }
    void issueBook() { string uid=readLine("User ID: "), bid=readLine("Book ID: "); User* u=findUser(uid); Book* b=findBook(bid); if(!u || !b) { cout << "Invalid user or book ID.\n"; return; } if(!b->available()) { cout << "Book is unavailable.\n"; return; } if(u->hasBook(bid)) { cout << "User already has this book.\n"; return; } b->issue(); u->issue(bid); save(); cout << "Book issued to " << u->getName() << ".\n"; }
    void returnBook() { string uid=readLine("User ID: "), bid=readLine("Book ID: "); User* u=findUser(uid); Book* b=findBook(bid); if(!u || !b || !u->returnBook(bid)) { cout << "This issue record was not found.\n"; return; } b->giveBack(); save(); cout << "Book returned successfully.\n"; }
    void viewIssued() const { bool any=false; for(const auto& u:users) if(!u.issued().empty()) { const Person& person = u; any=true; cout<<person.role()<<" "<<person.getId()<<" - "<<person.getName()<<": "; for(const auto& id:u.issued()) cout<<id<<' '; cout<<'\n'; } if(!any) cout<<"No books are currently issued.\n"; }
};

int main() {
    Library library; int choice;
    while(true) { cout << "\n====== LIBRARY MANAGEMENT SYSTEM ======\n1. Add book\n2. Remove book\n3. Update book\n4. View all books\n5. Register user\n6. Search books\n7. Issue book\n8. Return book\n9. View issued books\n0. Exit\nChoose: "; if(!(cin>>choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Enter a number.\n"; continue; } if(choice==0) { cout<<"Records saved. Goodbye.\n"; break; } switch(choice) { case 1: library.addBook(); break; case 2: library.removeBook(); break; case 3: library.updateBook(); break; case 4: library.viewBooks(); break; case 5: library.addUser(); break; case 6: library.searchBooks(); break; case 7: library.issueBook(); break; case 8: library.returnBook(); break; case 9: library.viewIssued(); break; default: cout<<"Invalid menu choice.\n"; } }
}

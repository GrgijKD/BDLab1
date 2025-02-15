#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

vector<int> publisherFreeList;

class Book {
public:
    static int nextId;
    int id;
    int publisherId;
    string title;
    string genre;
    string author;
    int deleted;

    Book(int publisherID, const string& title, const string& genre, const string& author)
        : id(nextId++), publisherId(publisherID), title(title), genre(genre), author(author), deleted(0) {
    }

    Book(int id_, int publisherID, const string& title, const string& genre, const string& author, int deletedFlag = 0)
        : id(id_), publisherId(publisherID), title(title), genre(genre), author(author), deleted(deletedFlag) {
    }

    int getId() const {
        return id;
    }

    int getPublisherId() const {
        return publisherId;
    }

    string getTitle() const {
        return title;
    }

    string getGenre() const {
        return genre;
    }

    string getAuthor() const {
        return author;
    }

    void print() const {
        cout << "Book ID: " << id << endl;
        cout << "Publisher ID: " << publisherId << endl;
        cout << "Title: " << title << endl;
        cout << "Genre: " << genre << endl;
        cout << "Author: " << author << endl;
        cout << "Deleted flag: " << deleted << endl;
    }
};

int Book::nextId = 0;

class Publisher {
public:
    static int nextId;
    int id;
    string name;
    string country;
    vector<Book> books;
    int deleted;
    vector<int> bookFreeList;

    Publisher(const string& publisherName, const string& publisherCountry)
        : id(nextId++), name(publisherName), country(publisherCountry), deleted(0) {
    }

    Publisher(int id_, const string& publisherName, const string& publisherCountry, int deletedFlag = 0)
        : id(id_), name(publisherName), country(publisherCountry), deleted(deletedFlag) {
    }

    int getId() const {
        return id;
    }

    string getName() const {
        return name;
    }

    string getCountry() const {
        return country;
    }

    const vector<Book>& getBooks() const {
        return books;
    }

    void addBook(const Book& book) {
        books.push_back(book);
    }

    void print() const {
        cout << "Publisher ID: " << id << endl;
        cout << "Name: " << name << endl;
        cout << "Country: " << country << endl;
        cout << "Deleted flag: " << deleted << endl;
        cout << "Books:" << endl << endl;
        for (const auto& book : books) {
            if (book.deleted == 0) {
                book.print();
                cout << endl;
            }
        }
    }
};

int Publisher::nextId = 0;

int findPublisherIndexById(const vector<Publisher>& publishers, int id) {
    for (size_t i = 0; i < publishers.size(); ++i) {
        if (publishers[i].getId() == id && publishers[i].deleted == 0) {
            return i;
        }
    }
    return -1;
}

int findBookIndexById(const vector<Book>& books, int bookId) {
    for (size_t i = 0; i < books.size(); ++i) {
        if (books[i].getId() == bookId && books[i].deleted == 0) {
            return i;
        }
    }
    return -1;
}

vector<Publisher> readFile() {
    vector<Publisher> publishers;
    ifstream masterFile("publishers.txt");
    ifstream slaveFile("books.txt");

    int maxPublisherId = 0;
    int maxBookId = 0;

    publisherFreeList.clear();

    if (masterFile.is_open() && slaveFile.is_open()) {
        string header;
        if (getline(masterFile, header)) {
        }
        string line;
        int index = 0;
        while (getline(masterFile, line)) {
            stringstream ss(line);
            string token;

            getline(ss, token, ';');
            int publisherId = stoi(token);
            maxPublisherId = max(maxPublisherId, publisherId);

            getline(ss, token, ';');
            string name = token;

            getline(ss, token, ';');
            string country = token;

            getline(ss, token, ';');
            int delFlag = stoi(token);

            Publisher publisher(publisherId, name, country, delFlag);
            publishers.push_back(publisher);
            if (delFlag == 1) {
                publisherFreeList.push_back(index);
            }
            index++;
        }

        string booksHeader;
        if (getline(slaveFile, booksHeader)) {
        }
        while (getline(slaveFile, line)) {
            stringstream ss(line);
            string token;

            getline(ss, token, ';');
            int bookId = stoi(token);
            maxBookId = max(maxBookId, bookId);

            getline(ss, token, ';');
            int publisherId = stoi(token);

            getline(ss, token, ';');
            string title = token;

            getline(ss, token, ';');
            string genre = token;

            getline(ss, token, ';');
            string author = token;

            getline(ss, token, ';');
            int delFlag = stoi(token);

            for (auto& publisher : publishers) {
                if (publisher.getId() == publisherId) {
                    Book book(bookId, publisherId, title, genre, author, delFlag);
                    publisher.addBook(book);
                    if (delFlag == 1) {
                        publisher.bookFreeList.push_back(publisher.books.size() - 1);
                    }
                    break;
                }
            }
        }

        masterFile.close();
        slaveFile.close();

        Publisher::nextId = maxPublisherId + 1;
        Book::nextId = maxBookId + 1;
    }
    else {
        cerr << "Error: Unable to open file for reading" << endl;
    }

    return publishers;
}

void writeFile(const vector<Publisher>& publishers) {
    ofstream masterFile("publishers.txt");
    ofstream slaveFile("books.txt");

    if (masterFile.is_open() && slaveFile.is_open()) {
        masterFile << "GARBAGE:";
        if (publisherFreeList.empty()) {
            masterFile << "-1";
        }
        else {
            for (size_t i = 0; i < publisherFreeList.size(); ++i) {
                masterFile << publisherFreeList[i];
                if (i != publisherFreeList.size() - 1)
                    masterFile << ",";
            }
        }
        masterFile << ";NEXT:" << publishers.size() << endl;

        for (const auto& publisher : publishers) {
            masterFile << publisher.getId() << ";"
                << publisher.getName() << ";"
                << publisher.getCountry() << ";"
                << publisher.deleted << endl;
        }

        vector<int> bookFreeGlobalIndices;
        int lineCounter = 0;
        vector<string> bookLines;
        for (const auto& publisher : publishers) {
            for (const auto& book : publisher.getBooks()) {
                ostringstream oss;
                oss << book.getId() << ";"
                    << book.getPublisherId() << ";"
                    << book.getTitle() << ";"
                    << book.getGenre() << ";"
                    << book.getAuthor() << ";"
                    << book.deleted;
                bookLines.push_back(oss.str());
                if (book.deleted == 1) {
                    bookFreeGlobalIndices.push_back(lineCounter);
                }
                lineCounter++;
            }
        }
        slaveFile << "GARBAGE:";
        if (bookFreeGlobalIndices.empty()) {
            slaveFile << "-1";
        }
        else {
            for (size_t i = 0; i < bookFreeGlobalIndices.size(); ++i) {
                slaveFile << bookFreeGlobalIndices[i];
                if (i != bookFreeGlobalIndices.size() - 1)
                    slaveFile << ",";
            }
        }
        slaveFile << ";NEXT:" << bookLines.size() << endl;
        for (const auto& line : bookLines) {
            slaveFile << line << endl;
        }

        masterFile.close();
        slaveFile.close();
    }
    else {
        cerr << "Error: Unable to open file for writing" << endl;
    }
}

void deletePublisher(vector<Publisher>& publishers, int id) {
    int foundIndex = -1;
    for (size_t i = 0; i < publishers.size(); ++i) {
        if (publishers[i].getId() == id && publishers[i].deleted == 0) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex != -1) {
        publishers[foundIndex].deleted = 1;
        publisherFreeList.push_back(foundIndex);

        for (size_t j = 0; j < publishers[foundIndex].books.size(); ++j) {
            if (publishers[foundIndex].books[j].deleted == 0) {
                publishers[foundIndex].books[j].deleted = 1;
                publishers[foundIndex].bookFreeList.push_back(j);
            }
        }
    }
    else {
        cerr << "Error: Invalid publisher id" << endl;
    }
}


void deleteBook(vector<Publisher>& publishers, int publisherId, int bookId) {
    int foundPublisherIndex = findPublisherIndexById(publishers, publisherId);
    if (foundPublisherIndex != -1) {
        vector<Book>& books = publishers[foundPublisherIndex].books;
        int foundBookIndex = -1;
        for (size_t i = 0; i < books.size(); ++i) {
            if (books[i].getId() == bookId && books[i].deleted == 0) {
                foundBookIndex = i;
                break;
            }
        }
        if (foundBookIndex != -1) {
            books[foundBookIndex].deleted = 1;
            publishers[foundPublisherIndex].bookFreeList.push_back(foundBookIndex);
        }
        else {
            cerr << "Error: Invalid book id" << endl;
        }
    }
    else {
        cerr << "Error: Invalid publisher id" << endl;
    }
}

void updatePublisherField(Publisher& publisher, const string& field, const string& value) {
    if (field == "name") {
        publisher.name = value;
    }
    else if (field == "country") {
        publisher.country = value;
    }
    else {
        cerr << "Error: Invalid field name" << endl;
    }
}

Book updateBookField(Book book, const string& field, const string& value) {
    if (field == "title") {
        book.title = value;
    }
    else if (field == "genre") {
        book.genre = value;
    }
    else if (field == "author") {
        book.author = value;
    }
    else {
        cerr << "Error: Invalid field name" << endl;
    }
    return book;
}

void countRecords(const vector<Publisher>& publishers) {
    int totalPublishers = 0;
    int totalBooks = 0;
    for (const auto& publisher : publishers) {
        if (publisher.deleted == 0)
            totalPublishers++;
        for (const auto& book : publisher.getBooks()) {
            if (book.deleted == 0)
                totalBooks++;
        }
    }
    cout << "Total publishers: " << totalPublishers << endl;
    cout << "Total books: " << totalBooks << endl;
}

void printFileContents(const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
    else {
        cerr << "Error: Unable to open file for reading" << endl;
    }
}

int main() {
    vector<Publisher> publishers = readFile();

    int choice;
    do {
        cout << "1) get-publisher" << endl;
        cout << "2) get-book" << endl;
        cout << "3) del-publisher" << endl;
        cout << "4) del-book" << endl;
        cout << "5) update-publisher" << endl;
        cout << "6) update-book" << endl;
        cout << "7) insert-publisher" << endl;
        cout << "8) insert-book" << endl;
        cout << "9) count all" << endl;
        cout << "10) count books in publisher" << endl;
        cout << "11) show publishers.txt" << endl;
        cout << "12) show books.txt" << endl;
        cout << "Enter your choice (0 to exit): ";
        cin >> choice;

        switch (choice) {
        case 1: {
            int id;
            cout << "Enter publisher id: ";
            cin >> id;
            int foundIndex = findPublisherIndexById(publishers, id);
            if (foundIndex != -1) {
                publishers[foundIndex].print();
                writeFile(publishers);
            }
            else {
                cerr << "Error: Invalid publisher id" << endl;
            }
            break;
        }
        case 2: {
            int bookId;
            cout << "Enter book id: ";
            cin >> bookId;
            bool found = false;
            for (const auto& publisher : publishers) {
                int foundBookIndex = findBookIndexById(publisher.books, bookId);
                if (foundBookIndex != -1) {
                    publisher.books[foundBookIndex].print();
                    writeFile(publishers);
                    found = true;
                    break;
                }
            }
            if (!found) {
                cerr << "Error: Invalid book id" << endl;
            }
            break;
        }
        case 3: {
            int id;
            cout << "Enter publisher id to delete: ";
            cin >> id;
            deletePublisher(publishers, id);
            writeFile(publishers);
            break;
        }
        case 4: {
            int publisherId, bookId;
            cout << "Enter publisher id: ";
            cin >> publisherId;
            cout << "Enter book id to delete: ";
            cin >> bookId;
            deleteBook(publishers, publisherId, bookId);
            writeFile(publishers);
            break;
        }
        case 5: {
            int id;
            string field, value;
            cout << "Enter publisher id to update: ";
            cin >> id;
            int foundIndex = findPublisherIndexById(publishers, id);
            if (foundIndex != -1) {
                cout << "Enter field to update (name/country): ";
                cin >> field;
                if (field == "name" || field == "country") {
                    cout << "Enter new value: ";
                    cin >> value;
                    updatePublisherField(publishers[foundIndex], field, value);
                    writeFile(publishers);
                }
                else {
                    cerr << "Error: Invalid field name" << endl;
                }
            }
            else {
                cerr << "Error: Invalid publisher id" << endl;
            }
            break;
        }
        case 6: {
            int publisherId, bookId;
            string field, value;
            cout << "Enter publisher id: ";
            cin >> publisherId;
            int foundPublisherIndex = findPublisherIndexById(publishers, publisherId);
            if (foundPublisherIndex != -1) {
                cout << "Enter book id to update: ";
                cin >> bookId;
                int foundBookIndex = findBookIndexById(publishers[foundPublisherIndex].books, bookId);
                if (foundBookIndex != -1) {
                    cout << "Enter field to update (title/genre/author): ";
                    cin >> field;
                    if (field == "title" || field == "genre" || field == "author") {
                        cout << "Enter new value: ";
                        cin >> value;
                        publishers[foundPublisherIndex].books[foundBookIndex] =
                            updateBookField(publishers[foundPublisherIndex].books[foundBookIndex], field, value);
                        writeFile(publishers);
                    }
                    else {
                        cerr << "Error: Invalid field name" << endl;
                    }
                }
                else {
                    cerr << "Error: Invalid book id" << endl;
                }
            }
            else {
                cerr << "Error: Invalid publisher id" << endl;
            }
            break;
        }
        case 7: {
            string name, country;
            cout << "Enter publisher name: ";
            cin >> name;
            cout << "Enter publisher country: ";
            cin >> country;
            if (!publisherFreeList.empty()) {
                int freeIndex = publisherFreeList.back();
                publisherFreeList.pop_back();
                publishers[freeIndex] = Publisher(name, country);
                cout << "Reused deleted slot for publisher " << name << endl;
            }
            else {
                publishers.emplace_back(name, country);
                cout << "Inserted publisher " << name << endl;
            }
            writeFile(publishers);
            break;
        }
        case 8: {
            int publisherId;
            string title, genre, author;
            cout << "Enter publisher id to add book: ";
            cin >> publisherId;
            cout << "Enter book title: ";
            cin >> title;
            cout << "Enter book genre: ";
            cin >> genre;
            cout << "Enter book author: ";
            cin >> author;
            int foundIndex = findPublisherIndexById(publishers, publisherId);
            if (foundIndex != -1) {
                Publisher& pub = publishers[foundIndex];
                if (!pub.bookFreeList.empty()) {
                    int freeIndex = pub.bookFreeList.back();
                    pub.bookFreeList.pop_back();
                    pub.books[freeIndex] = Book(publisherId, title, genre, author);
                    cout << "Reused deleted slot for book " << title << endl;
                }
                else {
                    pub.addBook(Book(publisherId, title, genre, author));
                    cout << "Inserted book " << title << endl;
                }
            }
            else {
                cerr << "Error: Invalid publisher id" << endl;
            }
            writeFile(publishers);
            break;
        }
        case 9: {
            countRecords(publishers);
            writeFile(publishers);
            break;
        }
        case 10: {
            int publisherId;
            cout << "Enter publisher id to count books: ";
            cin >> publisherId;
            int foundIndex = findPublisherIndexById(publishers, publisherId);
            if (foundIndex != -1) {
                int count = 0;
                for (const auto& book : publishers[foundIndex].books) {
                    if (book.deleted == 0)
                        count++;
                }
                cout << "Number of books in publisher " << publishers[foundIndex].name << ": " << count << endl;
                writeFile(publishers);
            }
            else {
                cerr << "Error: Invalid publisher id" << endl;
            }
            break;
        }
        case 11: {
            printFileContents("publishers.txt");
            writeFile(publishers);
            break;
        }
        case 12: {
            printFileContents("books.txt");
            writeFile(publishers);
            break;
        }
        case 0:
            cout << "Exiting..." << endl;
            writeFile(publishers);
            break;
        default:
            cerr << "Invalid choice. Enter a number between 0 and 12" << endl;
            writeFile(publishers);
        }
    } while (choice != 0);

    writeFile(publishers);

    return 0;
}
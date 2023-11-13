#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>


void syntax() {
	std::cerr
		<< "SYNTAX:\n"
		<< "torrent_dump <filesrc.torrent>\n";
	exit(EXIT_FAILURE);
}


void error(const std::string& msg) {
	std::cerr << "ERROR: " << msg << '\n';
	exit(EXIT_FAILURE);
}


class TorrentItem {
private:
	std::istream& is_;

public:
	TorrentItem(std::istream& is) : is_(is) {}

	void readExpectedChar(char expected) {
		using namespace std::literals;
		char c;
		if (!is_ || !is_.get(c) || c != expected) {
			error("Expected '"s + expected + "', found '" + c + "'");
		}
	}

	virtual std::ostream& print(std::ostream& os, int tabs) const = 0;

	virtual ~TorrentItem() = default;
};


std::unique_ptr<TorrentItem> nextTorrentItem(std::istream& is, const std::string& key = "");


class StringItem : public TorrentItem {
protected:
	std::string value_;

public:
	StringItem(std::istream& is) : TorrentItem(is) {
		size_t len;
		is >> len;
		readExpectedChar(':');
		value_.resize(len);
		if (!is.read(&value_[0], len)) {
			error("String content");
		}
	}

	std::ostream& print(std::ostream& os, int tab) const override {
		os << "\"";
		for (const auto& c : value_) {
			os << (32 <= c && c <= 126 ? c : '.');
		}
		os << "\"";
		return os;
	}

	operator std::string() const {
		return value_;
	}
};


class PiecesItem : public StringItem {
public:
	PiecesItem(std::istream& is) : StringItem(is) {}

	std::ostream& print(std::ostream& os, int tabs) const override {
		using namespace std;
		for (size_t i = 0; i < value_.size(); i++) {
			if (i % 20 == 0) {
				os << "\n" << std::string(tabs + 1, '\t');
			}
			// Ciascun byte è rappresentato con esattamente 2 caratteri hex 
			os << hex << setw(2) << setfill('0') << +static_cast<unsigned char>(value_[i]) << dec;
		}
		return os;
	}
};


class IntegerItem : public TorrentItem {
private:
	int64_t value_;

public:
	IntegerItem(std::istream& is) : TorrentItem(is) {
		readExpectedChar('i');
		is >> value_;
		readExpectedChar('e');
	}

	std::ostream& print(std::ostream& os, int tab) const override {
		os << value_;
		return os;
	}
};


class ListItem : public TorrentItem {
private:
	std::vector<std::unique_ptr<TorrentItem>> value_;

public:
	ListItem(std::istream& is) : TorrentItem(is) {
		readExpectedChar('l');
		while (is && is.peek() != 'e') {
			value_.push_back(nextTorrentItem(is));
		}
		readExpectedChar('e');
	}

	std::ostream& print(std::ostream& os, int tabs) const override {
		os << "[\n";
		for (const auto& v : value_) {
			os << std::string(tabs + 1, '\t');
			v->print(os, tabs + 1);
			os << "\n";
		}
		os << std::string(tabs, '\t') << "]";
		return os;
	}
};


class DictionaryItem : public TorrentItem {
private:
	std::map<std::string, std::unique_ptr<TorrentItem>> value_;

public:
	DictionaryItem(std::istream& is) : TorrentItem(is) {
		readExpectedChar('d');
		while (is && is.peek() != 'e') {
			StringItem key(is);
			value_[key] = nextTorrentItem(is, key);
		}
		readExpectedChar('e');
	}

	std::ostream& print(std::ostream& os, int tabs) const override {
		os << "{\n";
		for (const auto& el : value_) {
			std::cout << std::string(tabs + 1, '\t') << "\"" << el.first << "\" => ";
			el.second->print(os, tabs + 1);
			std::cout << "\n";
		}
		os << std::string(tabs, '\t') << "}";
		return os;
	}
};


std::unique_ptr<TorrentItem> nextTorrentItem(std::istream& is, const std::string& key) {
	char peek = is.peek();
	if (!is) {
		return nullptr;
	}
	switch (peek) {
	case 'd':
		return std::make_unique<DictionaryItem>(is);
		break;
	case 'i':
		return std::make_unique<IntegerItem>(is);
		break;
	case 'l':
		return std::make_unique<ListItem>(is);
		break;
	default:
		if (key == "pieces") 
			return std::make_unique<PiecesItem>(is);
		else
			return std::make_unique<StringItem>(is);
	}
}

int main(int argc, char** argv) {
	//{
		if (argc != 2) {
			syntax();
		}

		std::string srcfname = argv[1];
		std::ifstream is(srcfname, std::ios::binary);
		if (!is) {
			error("Failed to open file " + srcfname);
		}

		auto metainfo = nextTorrentItem(is);
		metainfo->print(std::cout, 0);
	//}
	//_CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}

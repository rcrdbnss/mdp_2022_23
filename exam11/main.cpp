#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <exception>
#include <unordered_map>

using namespace std;

#include "ppm.h"
#include "image_operations.h"

template <typename T>
T switch_endianness(T in, size_t n = sizeof(T)) {
	T ret = 0;
	for (int i = 0; i < n; i++) {
		ret = ret | (((in >> (8 * (n - 1 - i))) & 0xFF) << (8 * i));
	}
	return ret;
}

class UBJSONObject {
protected:
	std::string name_;
	std::vector<std::string> props_;
	// std::unordered_map<std::string, UBJSONObject> others_;

public:
	virtual ~UBJSONObject() {};
	virtual void set(std::string key, void* val) {
		if (key == "name") {
			name_ = std::string(reinterpret_cast<char*>(val));
		}
	}
	virtual void set(std::string key, UBJSONObject& obj) {
		// others_.insert({ key, obj });
	}
 	virtual unique_ptr<UBJSONObject> newChildObject(std::string key) {
		// UBJSONObject* ret = new UBJSONObject();
		auto ret = std::make_unique<UBJSONObject>();
		ret->name_ = key;
		return ret;
	};
	const auto& name() const { return name_; }
	const auto& props() const { return props_; }
	// const auto& others() const { return others_; }
};

class Image_ : public UBJSONObject {
protected:
	size_t width_, height_;
	size_t x_, y_;
	std::vector<vec3b> data_;

public:
	virtual ~Image_() override {};

	virtual void set(std::string key, void* val) override {
		if (key == "width") {
			width_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("width");
		}
		else if (key == "height") {
			height_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("height");
		}
		else if (key == "x") {
			x_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("x");
		}
		else if (key == "y") {
			y_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("y");
		}
		else if (key == "data") {
			vec3b* v = reinterpret_cast<vec3b*>(val);
			std::copy_n(v, width_ * height_, std::back_inserter(data_));
			props_.push_back("data");
		}
		else {
			UBJSONObject::set(key, val);
		}
	}

	virtual void set(std::string key, UBJSONObject& obj) {
		UBJSONObject::set(key, obj);
	}

	const auto& width() const { return width_; }
	const auto& height() const { return height_; }
	const auto& x() const { return x_; }
	const auto& y() const { return y_; }
	const auto& data() const { return data_; }
};

class Graphic_ : public UBJSONObject {
protected:
	size_t width_, height_;
	size_t x_, y_;
	vec3b border_color_, fill_color_;

public:
	virtual ~Graphic_() override {};

	virtual void set(std::string key, void* val) override {
		if (key == "width") {
			width_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("width");
		}
		else if (key == "height") {
			height_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("height");
		}
		else if (key == "x") {
			x_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("x");
		}
		else if (key == "y") {
			y_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("y");
		}
		else if (key == "border-color") {
			uint8_t* v = reinterpret_cast<uint8_t*>(val);
			border_color_ = vec3b(v[0], v[1], v[2]);
			props_.push_back("border-color");
		}
		else if (key == "fill-color") {
			uint8_t* v = reinterpret_cast<uint8_t*>(val);
			fill_color_ = vec3b(v[0], v[1], v[2]);
			props_.push_back("fill-color");
		}
		else {
			UBJSONObject::set(key, val);
		}
	}

	virtual void set(std::string key, UBJSONObject& obj) {
		UBJSONObject::set(key, obj);
	}

	const auto& width() const { return width_; }
	const auto& height() const { return height_; }
	const auto& x() const { return x_; }
	const auto& y() const { return y_; }
	const auto& border_color() const { return border_color_; }
	const auto& fill_color() const { return fill_color_; }
};

class Elements_ : public UBJSONObject {
protected:
	std::vector<Image_> images_;
	std::vector<Graphic_> graphics_;
	std::vector<std::string> all_;

public:
	virtual ~Elements_() override {};

	virtual void set(std::string key, void* val) override {
		if (key == "image") {
			Image_ img = *reinterpret_cast<Image_*>(val);
			images_.push_back(img);
		}
		else {
			UBJSONObject::set(key, val);
		}
	}

	virtual void set(std::string key, UBJSONObject& obj) {
		if (key == "image") {
			Image_ img = *reinterpret_cast<Image_*>(&obj);
			images_.push_back(img);
			all_.push_back("I");
		}
		else {
			Graphic_ gr = *reinterpret_cast<Graphic_*>(&obj);
			graphics_.push_back(gr);
			all_.push_back("G");
		}
	}

	virtual unique_ptr<UBJSONObject> newChildObject(std::string key) override {
		if (key == "image") { 
			// Image_* ret = new Image_();
			auto ret = std::make_unique<Image_>();
			ret->set("name", &key[0]);
			return ret;
		}
		else {
			// Graphic_* ret = new Graphic_();
			auto ret = std::make_unique<Graphic_>();
			ret->set("name", &key[0]);
			return ret;
		}
	};

	const auto& images() const { return images_; }
	const auto& graphics() const { return graphics_; }
	auto all() { 
		std::vector<UBJSONObject*> ret;
		size_t im = 0, gr = 0;
		for (size_t i = 0; i < all_.size(); i++) {
			if (all_[i] == "I") ret.push_back(&images_[im++]);
			else if (all_[i] == "G") ret.push_back(&graphics_[gr++]);
		}
		return ret; 
	}
};

class Canvas_ : public UBJSONObject {
protected:
	size_t width_, height_;
	vec3b background_;

public:
	virtual ~Canvas_() override {}

	virtual void set(std::string key, void* val) override {
		if (key == "width") {
			width_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("width");
		}
		else if (key == "height") {
			height_ = *reinterpret_cast<size_t*>(val);
			props_.push_back("height");
		}
		else if (key == "background") {
			uint8_t* v = reinterpret_cast<uint8_t*>(val);
			background_ = vec3b(v[0], v[1], v[2]);
			props_.push_back("backgroung");
		}
		else {
			UBJSONObject::set(key, val);
		}
	}

	virtual void set(std::string key, UBJSONObject& obj) {
		UBJSONObject::set(key, obj);
	}

	const auto& width() const { return width_; }
	const auto& height() const { return height_; }
	const auto& background() const { return background_; }
};

class Root_ : public UBJSONObject {
protected:
	Canvas_ canvas_;
	Elements_ elements_;

public:
	virtual ~Root_() override {}

	virtual void set(std::string key, void* val) override {
		if (key == "canvas") {
			canvas_ = *reinterpret_cast<Canvas_*>(val);
		}
		else if (key == "elements") {
			elements_ = *reinterpret_cast<Elements_*>(val);
		}
		else {
			UBJSONObject::set(key, val);
		}
	}

	virtual void set(std::string key, UBJSONObject& obj) {
		if (key == "canvas") {
			canvas_ = *reinterpret_cast<Canvas_*>(&obj);
		}
		else if (key == "elements") {
			elements_ = *reinterpret_cast<Elements_*>(&obj);
		}
		else {
			UBJSONObject::set(key, obj);
		}
	}

	virtual unique_ptr<UBJSONObject> newChildObject(std::string key) override {
		if (key == "canvas") { 
			// Canvas_* ret = new Canvas_();
			auto ret = std::make_unique<Canvas_>();
			ret->set("name", &key[0]);
			return ret;
		}
		else if (key == "elements") { 
			// Elements_* ret = new Elements_(); 
			auto ret = std::make_unique<Elements_>();
			ret->set("name", &key[0]);
			return ret;
		}
		else return UBJSONObject::newChildObject(key);
	};

	const auto& canvas() const { return canvas_; }
	const auto& elements() const { return elements_; }
};

class UBJSONParser {
private:
	std::ifstream& is_;

	void read_string(std::string& str) {
		if (is_.peek() == 'S') {
			is_.get();
		}
		size_t len;
		read_number(len);
		str.resize(len);
		is_.read(reinterpret_cast<char*>(&str[0]), len);
	}

	template <typename T>
	size_t read_number(T& val) {
		size_t size = 0;
		char ch = is_.get();
		switch (ch) {
		case 'i': {
			size = 1;
			int8_t v = 0;
			is_.read(reinterpret_cast<char*>(&v), size);
			val = T(v);
			break;
		}
		case 'U': {
			size = 1;
			uint8_t v = 0;
			is_.read(reinterpret_cast<char*>(&v), size);
			val = T(v);
			break;
		}
		case 'I': {
			size = 2;
			int16_t v = 0;
			is_.read(reinterpret_cast<char*>(&v), size);
			v = switch_endianness(v);
			val = T(v);
			break;
		}
		case 'l': {
			size = 4;
			int32_t v = 0;
			is_.read(reinterpret_cast<char*>(&v), size);
			v = switch_endianness(v);
			val = T(v);
			break;
		}
		case 'L': {
			size = 8;
			int64_t v = 0;
			is_.read(reinterpret_cast<char*>(&v), size);
			v = switch_endianness(v);
			val = T(v);
			break;
		}
		// float and double not implemented
		}
		return size;
	}

	void read_array(std::vector<uint8_t>& v, char& type) {
		if (is_.peek() == '[') is_.get();
		size_t size = 0;
		type = 0;
		if (is_.peek() == '$') {
			is_.get();
			is_.get(type);
		}
		if (is_.peek() == '#') {
			is_.get();
			read_number(size);
		}
		if (size == 0) {
			if (type == 0) {
				// canonical array, skip
				char c;
				do {
					is_.get(c);
				} while (c != ']');
			}
			else {
				// error
				std::cerr << "Error in reading an optimized numeric array, exiting...";
				exit(EXIT_FAILURE);
			}
		}
		else {
			if (type == 0) {
				// optimized mixed array, skip
				while (size-- > 0) {
					read_val();
				}
			}
			else if (type == 'U') {
				// optimized array with uint8 type
				while (size-- > 0) {
					uint8_t val;
					is_.read(reinterpret_cast<char*>(&val), 1);
					v.push_back(val);
				}
			}
		}
	}

	void* read_val() {
		void* val = 0;
		char c = is_.peek();
		switch (c) {
		case 'Z':
		case 'N':
		case 'T':
		case 'F':
			is_.get(c);
			break;

		case 'C':
			is_.get(c);
			is_.get(c);
			val = &c;

		case 'i':
		case 'U':
		case 'I':
		case 'l':
		case 'L':
		case 'd':
		case 'D': {
			auto num = new size_t[1];
			read_number(num[0]);
			val = &num[0];
			break;
		}

		case 'S': {
			auto str = new std::string();
			read_string(*str);
			val = &str[0];
			break;
		}

		case '[': {
			char type = 0;
			// read vector object
			auto vec = new std::vector<uint8_t>();
			read_array(*vec, type);
			// save the data for later (arr/val)
			const size_t size = vec->size();
			uint8_t* arr = new uint8_t[size];
			std::copy_n(vec->data(), size, arr);
			val = arr;
			// delete the vector
			delete(vec);
			break;
		}
		}
		return val;
	}

public:
	UBJSONParser(std::ifstream& is) : is_(is) {}

	~UBJSONParser() = default;

	template <typename JOBJ>
	void parse(JOBJ& obj) {
		char c;

		c = is_.peek();
		if (c == '{') {
			is_.get(c);	// '{'
		}
		
		while (is_.peek() != '}' && is_) {
			// read key
			std::string key;
			read_string(key);
			// read val
			c = is_.peek();
			if (c == '{') {
				unique_ptr<UBJSONObject> val = obj.newChildObject(key);
				// if (val > (UBJSONObject*)0) {
					parse(*val);
					val->set("name", &key[0]);
					obj.set(key, *val);
					// delete(val);
				// }
			}
			else {
				void* val = read_val();
				obj.set(key, val);
				if (val > (void*)0) delete(val);
			}
			c = is_.peek();
		}
		is_.get(); // skip '}'
	}
};

int convert(const string& sInput, const string& sOutput) {
	std::ifstream is(sInput, std::ios::binary);
	if (!is) {
		std::cerr << "Error 1";
		return EXIT_FAILURE;
	}

	std::ofstream os(sOutput, std::ios::binary);
	if (!os) {
		std::cerr << "Error 2";
		return EXIT_FAILURE;
	}

	UBJSONParser parser(is);
	Root_ root;
	parser.parse(root);

	Canvas_ canvas = root.canvas();

	// Dal file UBJ devo estrarre le informazioni e creare il canvas

	unsigned w = canvas.width();
	unsigned h = canvas.height();

	image<vec3b> img(w, h);
	std::fill(img.begin(), img.end(), canvas.background());
	writeP3("canvas.ppm", img);

	Elements_ elements = root.elements();
	std::vector<Image_> images = elements.images();
	size_t n = 1;
	for (auto& ei : images) {
		image<vec3b> im(ei.width(), ei.height());
		std::vector<vec3b> data = ei.data();
		std::copy(data.begin(), data.end(), im.begin());
		std::string fname = "image" + std::to_string(n) + ".ppm";
		writeP3(fname, im);

		paste<vec3b>(img, im, ei.x(), ei.y());

		n++;
	}

	for (const UBJSONObject* el : elements.all()) {
		std::cout << el->name() << " : ";
		for (auto& p : el->props()) {
			std::cout << p << ",";
		}
		std::cout << "\n";
	}

	// Output in formato PPM
	if (!writeP6(sOutput, img))
		return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	{
		string sInput = argv[1];
		string sOutput = argv[2];

		if (convert(sInput, sOutput) > 0) {
			return EXIT_FAILURE;
		}
	}
	_CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <bitset>
#include <string>
#include <stack>
#include <iterator>
#include <fstream>
using namespace std;

/**
 * Huffman Tree
 * ch stores current character
 * if ch is '$' [Esc], non leaf node
 * freq is node power
 * */
class Tree {
public:
	char ch;
	int freq;
	Tree * left;
	Tree * right;
	Tree(char ch, int freq) {
		this->left = this->right = nullptr;
		this->ch = ch;
		this->freq = freq;
	}
};

// tree compare for heapify
struct compare {
	bool operator()(Tree * A, Tree * B) {
		return A->freq > B->freq;
	}
};


/**
 * Converts Huffman Tree to postorder
 * 1 followed by leaf node character
 * 0 if non leaf node
 * */
string postorder(Tree * root) {
	if (!root) return "";
	string left = postorder(root->left);
	string right = postorder(root->right);

	if (root->ch != '$') {
		return left + right + "1" + root->ch;
	}
	return left + right + "0";
}

/**
 * Build huffman tree from postorder
 * for decompression
 * */
Tree * constructTree(string order) {
	stack<Tree *> st;
	for (int i = 0; i < order.size(); i++) {
		if (order[i] == '1') {
			i++;
			st.push(new Tree(order[i], 0));
		}
		else {
			if (st.size() == 1)
				break;
			Tree * x = st.top(); st.pop();
			Tree * y = st.top(); st.pop();

			Tree * Node = new Tree('$', 0);
			Node->right = x;
			Node->left = y;

			st.push(Node);
		}
	}

	return st.top();
}

string convertOrderToBits(string order) {
	string res = "";
	for (int i = 0; i < order.size() - 1; i++) {
		if (order[i] == '1') {
			string temp = bitset<8>((int)order[i + 1]).to_string();
			cout << order[i + 1] << " |  " << temp << endl;
			res += "1";
			res += temp;
		}
		else
			res += "0";
	}

	return res + "0";
}

string constructOrder(string orderbits) {
	string res = "";
	for (int i = 0; i < orderbits.size(); i++) {
		if (orderbits[i] == '1') {
			string next = orderbits.substr(i + 1, 8);
			i+= 9;
			char ch = (char)(stoi(next, nullptr, 2));
			res += "1";
			res += ch;
		}
		else {
			res += '0';
		}
	}
	return res;
}

// store huffman code in codes map
void getCodes(Tree * root, unordered_map<char, string> &codes, string curr){
 	if (!root) return;
 	// ASCII 27 Escape
 	if (root->ch != '$') {
 		codes[root->ch] = curr;
 		return;
 	}
 	getCodes(root->left, codes, curr + '0');
 	getCodes(root->right, codes, curr + '1');
}

int main(int argc, char * argv[]) {

	ifstream file;
	file.open("small.txt");

	string text = "";
	if (file.is_open()) {
		string line;
		while (getline(file, line)) {
			text += line;
			text += "\n";
		}
		text = text.substr(0, text.size() - 1);
	}
	unordered_map<char, int> freq_map;
	
	// Stage 1: convert text into frequency map
	for (char c: text) {
		freq_map[c]++;
	}

	Tree * node = new Tree('a', 45);
	// Stage 2: Convert frequency map to heap
	priority_queue<Tree *, vector<Tree *>, compare> heap;
	for (auto key: freq_map) {
		heap.push(new Tree(key.first, key.second));
	}

	Tree * top;
	while (heap.size() > 1) {
		Tree * left = heap.top(); heap.pop();
		Tree * right = heap.top(); heap.pop();

		// ASCII 27 Escape
		top = new Tree('$', left->freq + right->freq);
		top->left = left;
		top->right = right;

		heap.push(top);
	}

	string header = postorder(top);
	string header_bits = "";
	for (auto bits: header) {
		if (bits == '1' or bits == '0')
			header_bits += bits;
		header_bits += bitset<8>((int)bits).to_string();
	}



	unordered_map<char, string> codes;
	unordered_map<string, char> reverse_codes;
	getCodes(top, codes, "");

	for (auto k: freq_map) {
		reverse_codes[codes[k.first]] = k.first;
	}

	for (auto c: reverse_codes) {
		if (c.second == '\n') {
			cout << c.first << " "  << "\\n" << endl;
			continue;
		}
		cout << c.first << " " << c.second << endl;
	}

	string x = "", y = "";
	for (auto c: text) {
		x += bitset<8>((int)c).to_string();
		y += codes[c];
	}

	string AA = header;
	string AAtemp = convertOrderToBits(AA);
	cout << AA << " " << AAtemp << endl;
	string AAnew = constructOrder(AAtemp);
	cout << AAnew<< endl;

	// int count_header = header_bits.size();
	// int count_data = y.size();


	// cout << count_data << " " << count_header << " sizes" << endl;
	// y = bitset<32>(count_data).to_string() + bitset<32>(count_header).to_string() + header_bits + y;
	// int X = x.size();
	// int Y = y.size();

	// // cout << x << endl;
	// // cout << y << endl;
	// cout << "| X\t| Y\t|" << endl;
	// cout << "| " << X << " | " << Y << " |" << endl;
	// cout << "Compression Ratio: " << ((float)(X - Y) / X) * 100 << "%" << endl;

	int header_size = header_bits.size();
	int data_size = y.size();

	int current_size = header_size + data_size + 96;
	int padding = current_size % 16 == 0 ? 0 : 16 - (current_size % 16);
	cout << data_size << " " << header_size << " " << padding << endl;
	string encoded_text = "";
	encoded_text += bitset<32>(data_size).to_string();
	encoded_text += bitset<32>(header_size).to_string();
	encoded_text += bitset<32>(padding).to_string();

	encoded_text += header_bits;
	encoded_text += y;
	for (int i =0; i < padding; i++)
		encoded_text += "0";

	// writing to binary file
	ofstream output_file("exit.bin");
	string write_data = "";
	for (int i = 0; i < encoded_text.size(); i+= 4) {
		char curr_byte = (char)stoi(encoded_text.substr(i, 4), nullptr, 2);
		write_data += curr_byte;
	}

	output_file.write(write_data.c_str(), write_data.size());
	output_file.close();



	//decoding
	int Odata_size, Oheader_size, Opadding;
	ifstream read_file("exit.bin");
	string read_data = "";
	while (!read_file.eof()) {
		char a_code;
		read_file.read(&a_code, 1);
		read_data += bitset<4>((int)a_code).to_string();
	}
	read_data = encoded_text;
	
	Odata_size = (int)stoi(read_data.substr(0, 32), nullptr, 2);
	Oheader_size = (int)stoi(read_data.substr(32, 32), nullptr, 2);
	Opadding = (int)stoi(read_data.substr(64, 32), nullptr, 2);

	string Oheader = read_data.substr(96, Oheader_size);
	string Odata = read_data.substr(96 + Oheader_size, Odata_size);

	cout << header << " " << constructOrder(header_bits) << endl;

	unordered_map<char, string> decompression_map;
	Tree * newTree;
	getCodes(newTree, decompression_map, "");


	unordered_map<string, char> decompression_chars;
	for (auto k: decompression_map) {
		decompression_chars[decompression_map[k.first]] = k.first;
	}

	for (auto d: decompression_map) {
		cout << d.first << " " << d.second << endl;
	}
	string read_from_data = "";
	string w = "";
	for (int i =0; i < Odata_size; i++) {
		if (decompression_chars[w]) {
			read_from_data += decompression_chars[w];
			w = "";
		}
		else
			w += Odata[i];
	}

	cout << read_from_data << endl;
	return 0;
}

#include "./conftree.hpp"

ConfTree::ConfTree(){
    // std::cout << "default constructor called" << std::endl;
}

ConfTree::ConfTree(const ConfTree & other){

    // std::cout << "copy constructor called" << std::endl;
    *this = other;
}

ConfTree::~ConfTree(){
    // std::cout << "destructor called" << std::endl;
}

ConfTree& ConfTree::operator=(const ConfTree& other){
    // std::cout << "Copy assignment operator called" << std::endl;
    if (this != &other) {
        // test = other.getRawBits();
    }
    return *this;
}



int ConfTree::GetPort() {return Port;}
void ConfTree::SetPort(int Port) {this->Port = Port;}

std::string ConfTree::GetAddress() {return Address;}
void ConfTree::SetAddress(std::string Address) {this->Address = Address;}
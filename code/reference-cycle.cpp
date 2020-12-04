struct Person {
    std::shared_ptr<Person> partner;
};

int main(){
    std::shared_ptr<Person> alice = std::make_shared<Person>();
    std::shared_ptr<Person> bob = std::make_shared<Person>();
    alice->partner = bob;
    bob->partner = alice;
}

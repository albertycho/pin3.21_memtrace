#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <random>
#include <filesystem>

#define MIGRATION_LIMIT 2048
#define INPUT_BASE_NAME "2B_64K_4_2sharers_Phase"
#define OUTPUT_BASE_NAME "pagemaps_2B_2K_4_2sharers/";

std::map<uint64_t, uint64_t> read_page_owner(const std::string &dir) {
    std::ifstream file(dir + "/page_owner.txt");
    //std::ifstream file(dir + "/page_owner_CI.txt");
    uint64_t page_number, owner;
    std::map<uint64_t, uint64_t> page_map;

    while (file >> page_number >> owner) {
        page_map[page_number] = owner;
    }

    return page_map;
}
std::map<uint64_t, uint64_t> read_page_owner_CI(const std::string &dir) {
    //std::ifstream file(dir + "/page_owner.txt");
    std::ifstream file(dir + "/page_owner_CI.txt");
    uint64_t page_number, owner;
    std::map<uint64_t, uint64_t> page_map;

    while (file >> page_number >> owner) {
        page_map[page_number] = owner;
    }

    return page_map;
}

void write_page_owner(const std::map<uint64_t, uint64_t> &page_map, const std::string &dir) {
    std::ofstream file(dir + "/page_owner_pre.txt");
    for (const auto &pair : page_map) {
        file << pair.first << " " << pair.second << std::endl;
    }
}

int main() {
    int i = 0;
    std::string base = INPUT_BASE_NAME;
	std::string output_base = OUTPUT_BASE_NAME;

	std::filesystem::create_directory(output_base);
    //std::string base = "1B_512K_Phase";
    while (std::filesystem::exists(base + std::to_string(i+1))) {
        if(1){
            //////////PAGEMAP FOR BASELINE
            auto page_map1 = read_page_owner(base + std::to_string(i));
            auto page_map2 = read_page_owner(base + std::to_string(i+1));
            std::map<uint64_t, uint64_t> new_page_map;

            for (const auto &pair : page_map2) {
                uint64_t page_number = pair.first;
                uint64_t owner2 = pair.second;
                uint64_t owner1 = page_map1[page_number];

                if (owner1 == owner2) {
                    new_page_map[page_number] = owner2;
                } else {
                    uint64_t rand_val = rand() % 32;
                    if(rand_val <31) {
                    //uint64_t rand_val = rand() % 16;
                    //if(rand_val <15) {
                        // map to owner of Phase1
                        new_page_map[page_number] = owner2; // or owner2, based on randomness
                    } else {
                        //std::cout<<"does this happen?"<<std::endl;
                        // map to owner of Phase0
                        new_page_map[page_number] = owner1; // or owner2, based on randomness
                    }

                    //
                }
            }

            std::string new_dir = output_base+"phase" + std::to_string(i+1) + "_pagemaps";
            //std::string new_dir = "phase" + std::to_string(i+1) + "_pagemaps_CI";
            std::filesystem::create_directory(new_dir);
            write_page_owner(new_page_map, new_dir);
            std::filesystem::copy(base + std::to_string(i+1) + "/page_owner.txt", new_dir + "/page_owner_post.txt");
            //std::filesystem::copy(base + std::to_string(i+1) + "/page_owner_CI.txt", new_dir + "/page_owner_post.txt");
        }

        if(1){
                    //////////PAGEMAP FOR BASELINE
            auto page_map1 = read_page_owner_CI(base + std::to_string(i));
            auto page_map2 = read_page_owner_CI(base + std::to_string(i+1));
            std::map<uint64_t, uint64_t> new_page_map;

            for (const auto &pair : page_map2) {
                uint64_t page_number = pair.first;
                uint64_t owner2 = pair.second;
                uint64_t owner1 = page_map1[page_number];

                if (owner1 == owner2) {
                    new_page_map[page_number] = owner2;
                } else {
                    uint64_t rand_val = rand() % 32;
                    if(rand_val <31) {
                    //uint64_t rand_val = rand() % 16;
                    //if(rand_val <15) {
                        // map to owner of Phase1
                        new_page_map[page_number] = owner2; // or owner2, based on randomness
                    } else {
                        //std::cout<<"does this happen?"<<std::endl;
                        // map to owner of Phase0
                        new_page_map[page_number] = owner1; // or owner2, based on randomness
                    }

                    //
                }
            }

            //std::string new_dir = "phase" + std::to_string(i+1) + "_pagemaps";
            std::string new_dir = output_base+"phase" + std::to_string(i+1) + "_pagemaps_CI";
            std::filesystem::create_directory(new_dir);
            write_page_owner(new_page_map, new_dir);
            //std::filesystem::copy(base + std::to_string(i+1) + "/page_owner.txt", new_dir + "/page_owner_post.txt");
            std::filesystem::copy(base + std::to_string(i+1) + "/page_owner_CI.txt", new_dir + "/page_owner_post.txt");    
        }

        i++;
    }

    return 0;
}


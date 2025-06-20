#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

const std::string files[] = {
    "CCLocalLevels.dat", "CCGameManager.dat",
    "CCLocalLevels2.dat", "CCGameManager2.dat"
};

const int SLOT_COUNT = 9;

std::string get_env(const std::string& var) {
    char* buffer = nullptr;
    size_t size = 0;
    _dupenv_s(&buffer, &size, var.c_str());
    std::string value = buffer ? buffer : "";
    free(buffer);
    return value;
}

std::string get_gd_folder() {
    return get_env("LOCALAPPDATA") + "\\GeometryDash\\";
}

std::string get_base_backup_folder() {
    return get_env("USERPROFILE") + "\\OneDrive\\GD Saves\\";
}

std::string get_slot_path(int slot) {
    return get_base_backup_folder() + "Slot" + std::to_string(slot) + "\\";
}

std::string get_slot_label(int slot) {
    std::string label_path = get_slot_path(slot) + "slotname.txt";
    std::ifstream infile(label_path);
    std::string label;
    if (infile.is_open()) {
        std::getline(infile, label);
        infile.close();
    }
    return label.empty() ? "(Empty)" : label;
}

void write_slot_label(int slot, const std::string& label) {
    std::string label_path = get_slot_path(slot) + "slotname.txt";
    std::ofstream outfile(label_path);
    if (outfile.is_open()) {
        outfile << label;
        outfile.close();
    }
}

void list_slots() {
    std::cout << "\n=== Save Slots ===\n";
    for (int i = 1; i <= SLOT_COUNT; ++i) {
        std::cout << "[" << i << "] " << get_slot_label(i) << "\n";
    }
}

void copy_files(const std::string& from, const std::string& to) {
    for (const auto& file : files) {
        fs::path src = from + file;
        fs::path dst = to + file;
        if (fs::exists(src)) {
            try {
                fs::create_directories(to);
                fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
                std::cout << "Copied" << file << "\n";
            } catch (fs::filesystem_error& e) {
                std::cerr << "Error copying " << file << ": " << e.what() << "\n";
            }
        } else {
            std::cerr << "Missing file: " << file << "\n";
        }
    }
}

void delete_slot(int slot) {
    fs::path slot_path = get_slot_path(slot);
    if (!fs::exists(slot_path)) {
        std::cout << "Slot " << slot << " is already empty.\n";
        return;
    }
    try {
        fs::remove_all(slot_path);
        std::cout << "Deleted Slot " << slot << "\n";
    } catch (fs::filesystem_error& e) {
        std::cerr << "Failed to delete slot: " << e.what() << "\n";
    }
}

void rename_slot(int slot) {
    std::string new_name;
    std::cout << "Enter new name for Slot " << slot << ": ";
    std::cin.ignore();
    std::getline(std::cin, new_name);
    write_slot_label(slot, new_name);
    std::cout << "Renamed Slot " << slot << "\n";
}

void delete_current_gd_save() {
    std::string gd_path = get_gd_folder();
    bool deleted_any = false;
    for (const auto& file : files) {
        fs::path path = gd_path + file;
        if (fs::exists(path)) {
            try {
                fs::remove(path);
                std::cout << "Deleted " << file << "\n";
                deleted_any = true;
            } catch (...) {
                std::cerr << "Failed to delete " << file << "\n";
            }
        }
    }
    if (!deleted_any) {
        std::cout << "No save files to delete.\n";
    }
}

int main() {
    std::string gd_path = get_gd_folder();
    while (true) {
        list_slots();
        std::cout << "\n=== Menu ===\n";
        std::cout << "[1] Save current GD data to slot\n";
        std::cout << "[2] Load from slot to GD\n";
        std::cout << "[3] Delete slot\n";
        std::cout << "[4] Rename slot\n";
        std::cout << "[5] Remove current GD save\n";
        std::cout << "[6] Exit\n";
        std::cout << "Choose an option: ";
        int choice;
        std::cin >> choice;
        if (choice == 6) break;
        if (choice == 5) {
            delete_current_gd_save();
        } else {
            int slot;
            std::cout << "Enter slot number (1-" << SLOT_COUNT << "): ";
            std::cin >> slot;
            if (slot < 1 || slot > SLOT_COUNT) {
                std::cout << "Invalid slot number.\n";
                continue;
            }
            std::string slot_path = get_slot_path(slot);
            switch (choice) {
                case 1:
                    copy_files(gd_path, slot_path);
                    std::cout << "Enter a name for this save slot: ";
                    std::cin.ignore();
                    {
                        std::string label;
                        std::getline(std::cin, label);
                        write_slot_label(slot, label);
                    }
                    break;
                case 2:
                    delete_current_gd_save();
                    copy_files(slot_path, gd_path);
                    break;
                case 3:
                    delete_slot(slot);
                    break;
                case 4:
                    rename_slot(slot);
                    break;
                default:
                    std::cout << "Invalid choice.\n";
            }
        }
        std::cout << "\n";
        system("pause");
        system("cls");
    }
    return 0;
}

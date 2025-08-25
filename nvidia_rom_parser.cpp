#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstring>
#include <map>
#include <memory>

// BIT Header structure
#pragma pack(push, 1)
struct BITHeader {
    uint16_t id;                // 0xB8FF
    uint32_t signature;         // "BIT\0"
    uint16_t bcdVersion;        // BCD Version
    uint8_t headerSize;         // Header size in bytes
    uint8_t tokenSize;          // Token size in bytes
    uint8_t tokenEntries;       // Number of token entries
    uint8_t checksum;           // Header checksum
};

struct BITToken {
    uint8_t id;                 // Token identifier
    uint8_t dataVersion;        // Data structure version
    uint16_t dataSize;          // Size of data structure
    uint16_t dataPointer;       // Pointer to data structure
};

// Various data structures according to spec
struct BIOSData_v1 {
    uint32_t biosVersion;
    uint8_t biosOEMVersion;
    uint8_t biosChecksum;
    uint16_t int15PostCallbacks;
    uint16_t int15SystemCallbacks;
    uint16_t biosBoardId;
    uint16_t frameCount;
    uint32_t biosmodDate : 24;  // 24-bit field
};

struct BIOSData_v2 {
    uint32_t biosVersion;
    uint8_t biosOEMVersion;
    uint8_t biosChecksum;
    uint16_t int15PostCallbacks;
    uint16_t int15SystemCallbacks;
    uint16_t frameCount;
    uint32_t reserved;
    uint8_t maxHeadsAtPost;
    uint8_t memorySizeReport;
    uint8_t hScaleFactor;
    uint8_t vScaleFactor;
    uint16_t dataTablePointer;
    uint16_t rompacksPointer;
    uint16_t appliedRompacksPointer;
    uint8_t appliedRompackMax;
    uint8_t appliedRompackCount;
    uint8_t moduleMapExternal0;
    uint32_t compressionInfoPointer;
};

struct StringPtrs_v1 {
    uint16_t signOnMessagePtr;
    uint8_t signOnMessageMaxLength;
    uint16_t oemString;
    uint8_t oemStringSize;
    uint16_t oemVendorName;
    uint8_t oemVendorNameSize;
    uint16_t oemProductName;
    uint8_t oemProductNameSize;
    uint16_t oemProductRevision;
    uint8_t oemProductRevisionSize;
};

struct StringPtrs_v2 {
    uint16_t signOnMessagePtr;
    uint8_t signOnMessageMaxLength;
    uint16_t versionString;
    uint8_t versionStringSize;
    uint16_t copyrightString;
    uint8_t copyrightStringSize;
    uint16_t oemString;
    uint8_t oemStringSize;
    uint16_t oemVendorName;
    uint8_t oemVendorNameSize;
    uint16_t oemProductName;
    uint8_t oemProductNameSize;
    uint16_t oemProductRevision;
    uint8_t oemProductRevisionSize;
};
#pragma pack(pop)

class NVIDIAROMParser {
private:
    std::vector<uint8_t> romData;
    std::string filename;
    std::ofstream* outputFile;
    
    // Token name mapping
    std::map<uint8_t, std::string> tokenNames = {
        {0x32, "BIT_TOKEN_I2C_PTRS"},
        {0x41, "BIT_TOKEN_DAC_PTRS"},
        {0x42, "BIT_TOKEN_BIOSDATA"},
        {0x43, "BIT_TOKEN_CLOCK_PTRS"},
        {0x44, "BIT_TOKEN_DFP_PTRS"},
        {0x49, "BIT_TOKEN_NVINIT_PTRS"},
        {0x4C, "BIT_TOKEN_LVDS_PTRS"},
        {0x4D, "BIT_TOKEN_MEMORY_PTRS"},
        {0x4E, "BIT_TOKEN_NOP"},
        {0x50, "BIT_TOKEN_PERF_PTRS"},
        {0x52, "BIT_TOKEN_BRIDGE_FW_DATA"},
        {0x53, "BIT_TOKEN_STRING_PTRS"},
        {0x54, "BIT_TOKEN_TMDS_PTRS"},
        {0x55, "BIT_TOKEN_DISPLAY_PTRS"},
        {0x56, "BIT_TOKEN_VIRTUAL_PTRS"},
        {0x63, "BIT_TOKEN_32BIT_PTRS"},
        {0x64, "BIT_TOKEN_DP_PTRS"},
        {0x70, "BIT_TOKEN_FALCON_DATA"},
        {0x75, "BIT_TOKEN_UEFI_DATA"},
        {0x78, "BIT_TOKEN_MXM_DATA"}
    };

public:
    NVIDIAROMParser(const std::string& fname) : filename(fname), outputFile(nullptr) {}
    
    ~NVIDIAROMParser() {
        if (outputFile && outputFile->is_open()) {
            outputFile->close();
        }
    }

    bool loadROM() {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        romData.resize(size);
        file.read(reinterpret_cast<char*>(romData.data()), size);
        
        if (!file) {
            std::cerr << "Error: Could not read file " << filename << std::endl;
            return false;
        }

        return true;
    }

    void setOutputFile(const std::string& outFilename) {
        outputFile = new std::ofstream(outFilename);
        if (!outputFile->is_open()) {
            delete outputFile;
            outputFile = nullptr;
            std::cerr << "Warning: Could not open output file " << outFilename << std::endl;
        }
    }

    void output(const std::string& text) {
        std::cout << text;
        if (outputFile && outputFile->is_open()) {
            *outputFile << text;
        }
    }

    size_t findPCIExpansionROM() {
        // Look for PCI Expansion ROM signature {0x55, 0xAA} on 512-byte boundaries
        for (size_t i = 0; i < romData.size() - 1; i += 512) {
            if (romData[i] == 0x55 && romData[i + 1] == 0xAA) {
                // Verify PCIR signature at offset indicated by bytes 24-25
                if (i + 25 < romData.size()) {
                    uint16_t pcirOffset = *reinterpret_cast<uint16_t*>(&romData[i + 24]);
                    if (i + pcirOffset + 4 < romData.size()) {
                        if (std::memcmp(&romData[i + pcirOffset], "PCIR", 4) == 0) {
                            return i;
                        }
                    }
                }
            }
        }
        return 0; // Default to start if not found
    }

    size_t findBITHeader(size_t startOffset = 0) {
        const uint16_t BIT_ID = 0xB8FF;
        const uint32_t BIT_SIGNATURE = 0x00544942; // "BIT\0" in little-endian

        for (size_t i = startOffset; i <= romData.size() - sizeof(BITHeader); ++i) {
            const BITHeader* header = reinterpret_cast<const BITHeader*>(&romData[i]);
            if (header->id == BIT_ID && header->signature == BIT_SIGNATURE) {
                // Verify checksum
                uint8_t sum = 0;
                for (size_t j = 0; j < header->headerSize; ++j) {
                    sum += romData[i + j];
                }
                if (sum == 0) {
                    return i;
                }
            }
        }
        return SIZE_MAX; // Not found
    }

    std::string readString(uint16_t offset, uint8_t maxLength = 255) {
        if (offset == 0 || offset >= romData.size()) return "NULL";
        
        std::string result;
        for (size_t i = offset; i < romData.size() && i < offset + maxLength; ++i) {
            if (romData[i] == 0) break;
            result += static_cast<char>(romData[i]);
        }
        return result.empty() ? "NULL" : result;
    }

    void dumpHex(const uint8_t* data, size_t size, const std::string& prefix = "") {
        for (size_t i = 0; i < size; ++i) {
            if (i % 16 == 0 && i > 0) output("\n");
            if (i % 16 == 0) output(prefix);
            
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[i]) << " ";
            output(ss.str());
        }
        output("\n");
    }

    void parseBIOSData(uint16_t offset, uint8_t version, uint16_t size) {
        output("    BIOS Data (Version " + std::to_string(version) + "):\n");
        
        if (version == 1 && size >= sizeof(BIOSData_v1)) {
            const BIOSData_v1* data = reinterpret_cast<const BIOSData_v1*>(&romData[offset]);
            
            std::stringstream ss;
            ss << "      BIOS Version: " << std::hex << data->biosVersion << "\n";
            ss << "      BIOS OEM Version: " << static_cast<int>(data->biosOEMVersion) << "\n";
            ss << "      BIOS Checksum: 0x" << std::hex << static_cast<int>(data->biosChecksum) << "\n";
            ss << "      INT15 POST Callbacks: 0x" << std::hex << data->int15PostCallbacks << "\n";
            ss << "      INT15 System Callbacks: 0x" << std::hex << data->int15SystemCallbacks << "\n";
            ss << "      BIOS Board ID: 0x" << std::hex << data->biosBoardId << "\n";
            ss << "      Frame Count: " << std::dec << data->frameCount << "\n";
            ss << "      BIOSMOD Date: " << std::hex << data->biosmodDate << "\n";
            output(ss.str());
        } else if (version == 2 && size >= sizeof(BIOSData_v2)) {
            const BIOSData_v2* data = reinterpret_cast<const BIOSData_v2*>(&romData[offset]);
            
            std::stringstream ss;
            ss << "      BIOS Version: " << std::hex << data->biosVersion << "\n";
            ss << "      BIOS OEM Version: " << static_cast<int>(data->biosOEMVersion) << "\n";
            ss << "      BIOS Checksum: 0x" << std::hex << static_cast<int>(data->biosChecksum) << "\n";
            ss << "      INT15 POST Callbacks: 0x" << std::hex << data->int15PostCallbacks << "\n";
            ss << "      INT15 System Callbacks: 0x" << std::hex << data->int15SystemCallbacks << "\n";
            ss << "      Frame Count: " << std::dec << data->frameCount << "\n";
            ss << "      Max Heads at POST: " << static_cast<int>(data->maxHeadsAtPost) << "\n";
            ss << "      Memory Size Report: " << static_cast<int>(data->memorySizeReport) << "\n";
            ss << "      H Scale Factor: " << static_cast<int>(data->hScaleFactor) << "\n";
            ss << "      V Scale Factor: " << static_cast<int>(data->vScaleFactor) << "\n";
            ss << "      Data Table Pointer: 0x" << std::hex << data->dataTablePointer << "\n";
            ss << "      ROMpacks Pointer: 0x" << std::hex << data->rompacksPointer << "\n";
            ss << "      Applied ROMpacks Pointer: 0x" << std::hex << data->appliedRompacksPointer << "\n";
            ss << "      Applied ROMpack Max: " << static_cast<int>(data->appliedRompackMax) << "\n";
            ss << "      Applied ROMpack Count: " << static_cast<int>(data->appliedRompackCount) << "\n";
            ss << "      Module Map External 0: 0x" << std::hex << static_cast<int>(data->moduleMapExternal0) << "\n";
            ss << "      Compression Info Pointer: 0x" << std::hex << data->compressionInfoPointer << "\n";
            output(ss.str());
        }
    }

    void parseStringPtrs(uint16_t offset, uint8_t version, uint16_t size) {
        output("    String Pointers (Version " + std::to_string(version) + "):\n");
        
        if (version == 1 && size >= sizeof(StringPtrs_v1)) {
            const StringPtrs_v1* data = reinterpret_cast<const StringPtrs_v1*>(&romData[offset]);
            
            output("      Sign On Message: \"" + readString(data->signOnMessagePtr, data->signOnMessageMaxLength) + "\"\n");
            output("      OEM String: \"" + readString(data->oemString, data->oemStringSize) + "\"\n");
            output("      OEM Vendor Name: \"" + readString(data->oemVendorName, data->oemVendorNameSize) + "\"\n");
            output("      OEM Product Name: \"" + readString(data->oemProductName, data->oemProductNameSize) + "\"\n");
            output("      OEM Product Revision: \"" + readString(data->oemProductRevision, data->oemProductRevisionSize) + "\"\n");
            
        } else if (version == 2 && size >= sizeof(StringPtrs_v2)) {
            const StringPtrs_v2* data = reinterpret_cast<const StringPtrs_v2*>(&romData[offset]);
            
            output("      Sign On Message: \"" + readString(data->signOnMessagePtr, data->signOnMessageMaxLength) + "\"\n");
            output("      Version String: \"" + readString(data->versionString, data->versionStringSize) + "\"\n");
            output("      Copyright String: \"" + readString(data->copyrightString, data->copyrightStringSize) + "\"\n");
            output("      OEM String: \"" + readString(data->oemString, data->oemStringSize) + "\"\n");
            output("      OEM Vendor Name: \"" + readString(data->oemVendorName, data->oemVendorNameSize) + "\"\n");
            output("      OEM Product Name: \"" + readString(data->oemProductName, data->oemProductNameSize) + "\"\n");
            output("      OEM Product Revision: \"" + readString(data->oemProductRevision, data->oemProductRevisionSize) + "\"\n");
        }
    }

    void parseGenericPointers(uint16_t offset, uint16_t size, const std::string& tokenName) {
        output("    " + tokenName + " Data:\n");
        
        // Generic parsing - show raw data as hex dump
        if (offset + size <= romData.size()) {
            output("      Raw Data (hex):\n");
            dumpHex(&romData[offset], size, "        ");
        } else {
            output("      Error: Data extends beyond ROM boundary\n");
        }
    }

    bool parse() {
        if (romData.empty()) {
            std::cerr << "Error: No ROM data loaded" << std::endl;
            return false;
        }

        output("NVIDIA ROM File Analysis\n");
        output("========================\n\n");
        output("File: " + filename + "\n");
        output("Size: " + std::to_string(romData.size()) + " bytes\n\n");

        // Find PCI Expansion ROM
        size_t pciRomOffset = findPCIExpansionROM();
        output("PCI Expansion ROM found at offset: 0x" + 
               (std::stringstream() << std::hex << pciRomOffset).str() + "\n\n");

        // Find BIT Header
        size_t bitHeaderOffset = findBITHeader(pciRomOffset);
        if (bitHeaderOffset == SIZE_MAX) {
            output("Error: BIT Header not found\n");
            return false;
        }

        output("BIT Header found at offset: 0x" + 
               (std::stringstream() << std::hex << bitHeaderOffset).str() + "\n\n");

        const BITHeader* header = reinterpret_cast<const BITHeader*>(&romData[bitHeaderOffset]);

        // Parse BIT Header
        output("BIT Header:\n");
        std::stringstream ss;
        ss << "  ID: 0x" << std::hex << header->id << "\n";
        ss << "  Signature: \"" << std::string(reinterpret_cast<const char*>(&header->signature), 4) << "\"\n";
        ss << "  BCD Version: 0x" << std::hex << header->bcdVersion << "\n";
        ss << "  Header Size: " << std::dec << static_cast<int>(header->headerSize) << " bytes\n";
        ss << "  Token Size: " << static_cast<int>(header->tokenSize) << " bytes\n";
        ss << "  Token Entries: " << static_cast<int>(header->tokenEntries) << "\n";
        ss << "  Checksum: 0x" << std::hex << static_cast<int>(header->checksum) << "\n\n";
        output(ss.str());

        // Parse BIT Tokens
        output("BIT Tokens:\n");
        size_t tokenOffset = bitHeaderOffset + header->headerSize;
        
        for (int i = 0; i < header->tokenEntries; ++i) {
            if (tokenOffset + sizeof(BITToken) > romData.size()) {
                output("Error: Token " + std::to_string(i) + " extends beyond ROM boundary\n");
                break;
            }

            const BITToken* token = reinterpret_cast<const BITToken*>(&romData[tokenOffset]);
            
            std::string tokenName = tokenNames.count(token->id) ? 
                                  tokenNames[token->id] : 
                                  "UNKNOWN_TOKEN";
            
            output("  Token " + std::to_string(i) + ": " + tokenName + " (0x" + 
                   (std::stringstream() << std::hex << static_cast<int>(token->id)).str() + ")\n");
            output("    Data Version: " + std::to_string(token->dataVersion) + "\n");
            output("    Data Size: " + std::to_string(token->dataSize) + " bytes\n");
            output("    Data Pointer: 0x" + 
                   (std::stringstream() << std::hex << token->dataPointer).str() + "\n");

            // Parse specific token data
            if (token->dataPointer != 0 && token->dataSize > 0) {
                switch (token->id) {
                    case 0x42: // BIT_TOKEN_BIOSDATA
                        parseBIOSData(token->dataPointer, token->dataVersion, token->dataSize);
                        break;
                    case 0x53: // BIT_TOKEN_STRING_PTRS
                        parseStringPtrs(token->dataPointer, token->dataVersion, token->dataSize);
                        break;
                    case 0x4E: // BIT_TOKEN_NOP
                        output("    No Operation Token (NOP)\n");
                        break;
                    default:
                        parseGenericPointers(token->dataPointer, token->dataSize, tokenName);
                        break;
                }
            } else {
                output("    NULL pointer or zero size - no data\n");
            }
            output("\n");

            tokenOffset += header->tokenSize;
        }

        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file> [output_file]" << std::endl;
        std::cerr << "  rom_file: Path to the .rom file to parse" << std::endl;
        std::cerr << "  output_file: Optional output text file" << std::endl;
        return 1;
    }

    std::string romFile = argv[1];
    
    // Verify file extension
    if (romFile.length() < 4 || romFile.substr(romFile.length() - 4) != ".rom") {
        std::cerr << "Warning: File does not have .rom extension" << std::endl;
    }

    NVIDIAROMParser parser(romFile);
    
    if (argc >= 3) {
        parser.setOutputFile(argv[2]);
    }

    if (!parser.loadROM()) {
        return 1;
    }

    if (!parser.parse()) {
        return 1;
    }

    std::cout << "\nParsing completed successfully!" << std::endl;
    return 0;
}

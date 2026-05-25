#include <fstream>

// Export raw data for analysis
// Format: header (nx, ny) followed by raw sequence of fractal_el
void export_data_raw(const fractal_pl& plane, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    
    // Write dimensions so Python knows how to reshape the array
    std::size_t nx = plane.nx();
    std::size_t ny = plane.ny();
    ofs.write(reinterpret_cast<const char*>(&nx), sizeof(std::size_t));
    ofs.write(reinterpret_cast<const char*>(&ny), sizeof(std::size_t));
    
    // Write data
    const auto& data = plane.data();
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(fractal_el));
}

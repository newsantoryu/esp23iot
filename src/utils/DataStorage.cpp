#include "DataStorage.h"
#include "Logger.h"
#include <FS.h>

DataStorage::DataStorage(const String& filename) 
    : _filename(filename), _bufferIndex(0), _initialized(false) {
}

bool DataStorage::initialize() {
    if (!LittleFS.begin()) {
        Logger::error("Failed to initialize LittleFS");
        return false;
    }
    
    // Criar diretório de dados se não existir
    if (!LittleFS.exists("/data")) {
        if (!LittleFS.mkdir("/data")) {
            Logger::error("Failed to create /data directory");
            return false;
        }
    }
    
    // Verificar se arquivo existe
    if (!LittleFS.exists(_filename)) {
        File file = LittleFS.open(_filename, "w");
        if (!file) {
            Logger::error("Failed to create log file: " + _filename);
            return false;
        }
        
        // Escrever cabeçalho CSV
        file.println("timestamp,frequency,note,octave,cents,confidence,stable,snr,crossings");
        file.close();
        Logger::info("Created new log file: " + _filename);
    }
    
    _initialized = true;
    Logger::info("DataStorage initialized successfully");
    return true;
}

bool DataStorage::save(const LogEntry& entry) {
    if (!_initialized) {
        Logger::error("DataStorage not initialized");
        return false;
    }
    
    // Adicionar ao buffer circular
    _buffer[_bufferIndex] = entry;
    _bufferIndex = (_bufferIndex + 1) % BUFFER_SIZE;
    
    // Salvar em arquivo
    File file = LittleFS.open(_filename, "a");
    if (!file) {
        Logger::error("Failed to open log file for writing");
        return false;
    }
    
    String csvLine = formatLogEntry(entry);
    file.println(csvLine);
    file.close();
    
    // Verificar necessidade de rotação
    rotateLogsIfNeeded();
    
    return true;
}

String DataStorage::formatLogEntry(const LogEntry& entry) {
    return String(entry.timestamp) + "," +
           String(entry.frequency, 2) + "," +
           entry.note + "," +
           String(entry.octave) + "," +
           String(entry.cents, 1) + "," +
           String(entry.confidence, 3) + "," +
           String(entry.stable ? 1 : 0) + "," +
           String(entry.snr, 3) + "," +
           String(entry.crossings);
}

LogEntry DataStorage::parseCSVLine(const String& csvLine) {
    LogEntry entry;
    int start = 0;
    int end = csvLine.indexOf(',');
    
    // Skip header line
    if (csvLine.startsWith("timestamp")) {
        return entry;
    }
    
    // Parse timestamp
    if (end > 0) {
        entry.timestamp = csvLine.substring(start, end).toInt();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse frequency
    if (end > 0) {
        entry.frequency = csvLine.substring(start, end).toFloat();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse note
    if (end > 0) {
        entry.note = csvLine.substring(start, end);
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse octave
    if (end > 0) {
        entry.octave = csvLine.substring(start, end).toInt();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse cents
    if (end > 0) {
        entry.cents = csvLine.substring(start, end).toFloat();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse confidence
    if (end > 0) {
        entry.confidence = csvLine.substring(start, end).toFloat();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse stable
    if (end > 0) {
        entry.stable = csvLine.substring(start, end).toInt() == 1;
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse SNR
    if (end > 0) {
        entry.snr = csvLine.substring(start, end).toFloat();
        start = end + 1;
        end = csvLine.indexOf(',', start);
    }
    
    // Parse crossings
    if (end > 0) {
        entry.crossings = csvLine.substring(start).toInt();
    }
    
    return entry;
}

std::vector<LogEntry> DataStorage::query(const String& filter, int limit) {
    std::vector<LogEntry> results;
    
    if (!_initialized) {
        Logger::error("DataStorage not initialized");
        return results;
    }
    
    File file = LittleFS.open(_filename, "r");
    if (!file) {
        Logger::error("Failed to open log file for reading");
        return results;
    }
    
    // Skip header
    String line = file.readStringUntil('\n');
    
    // Read lines and apply filter
    while (file.available() && results.size() < limit) {
        line = file.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0) continue;
        
        LogEntry entry = parseCSVLine(line);
        
        // Apply simple filter
        bool matches = true;
        if (filter.length() > 0) {
            if (filter.startsWith("frequency>")) {
                float threshold = filter.substring(10).toFloat();
                matches = (entry.frequency > threshold);
            } else if (filter.startsWith("note=")) {
                String noteFilter = filter.substring(5);
                matches = (entry.note == noteFilter);
            } else if (filter.startsWith("octave=")) {
                int octaveFilter = filter.substring(7).toInt();
                matches = (entry.octave == octaveFilter);
            } else if (filter.startsWith("stable=")) {
                bool stableFilter = filter.substring(7).toInt() == 1;
                matches = (entry.stable == stableFilter);
            }
        }
        
        if (matches) {
            results.push_back(entry);
        }
    }
    
    file.close();
    return results;
}

std::vector<LogEntry> DataStorage::getRecentLogs(int count) {
    std::vector<LogEntry> results;
    
    if (!_initialized) {
        Logger::error("DataStorage not initialized");
        return results;
    }
    
    // Obter logs do buffer circular (mais recentes primeiro)
    for (int i = 0; i < count && i < BUFFER_SIZE; i++) {
        int index = (_bufferIndex - 1 - i + BUFFER_SIZE) % BUFFER_SIZE;
        if (_buffer[index].timestamp > 0) {
            results.push_back(_buffer[index]);
        }
    }
    
    return results;
}

bool DataStorage::clear() {
    if (!_initialized) {
        Logger::error("DataStorage not initialized");
        return false;
    }
    
    // Clear file
    File file = LittleFS.open(_filename, "w");
    if (!file) {
        Logger::error("Failed to clear log file");
        return false;
    }
    
    file.println("timestamp,frequency,note,octave,cents,confidence,stable,snr,crossings");
    file.close();
    
    // Clear buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        _buffer[i] = LogEntry();
    }
    _bufferIndex = 0;
    
    Logger::info("Log data cleared successfully");
    return true;
}

bool DataStorage::exportCSV(const String& filename) {
    if (!_initialized) {
        Logger::error("DataStorage not initialized");
        return false;
    }
    
    File source = LittleFS.open(_filename, "r");
    File dest = LittleFS.open(filename, "w");
    
    if (!source || !dest) {
        Logger::error("Failed to open files for export");
        if (source) source.close();
        if (dest) dest.close();
        return false;
    }
    
    // Copy file content
    while (source.available()) {
        dest.write(source.read());
    }
    
    source.close();
    dest.close();
    
    Logger::info("Logs exported to: " + filename);
    return true;
}

String DataStorage::getStatistics() {
    if (!_initialized) {
        return "DataStorage not initialized";
    }
    
    std::vector<LogEntry> allLogs = query("", 1000);
    
    if (allLogs.empty()) {
        return "No logs available";
    }
    
    // Calculate statistics
    float totalFreq = 0;
    int stableCount = 0;
    float totalConfidence = 0;
    float minFreq = allLogs[0].frequency;
    float maxFreq = allLogs[0].frequency;
    
    for (const auto& entry : allLogs) {
        totalFreq += entry.frequency;
        totalConfidence += entry.confidence;
        if (entry.stable) stableCount++;
        if (entry.frequency < minFreq) minFreq = entry.frequency;
        if (entry.frequency > maxFreq) maxFreq = entry.frequency;
    }
    
    float avgFreq = totalFreq / allLogs.size();
    float avgConfidence = totalConfidence / allLogs.size();
    float stabilityRate = (float)stableCount / allLogs.size() * 100.0;
    
    String stats = "Total Logs: " + String(allLogs.size()) + "\n";
    stats += "Avg Frequency: " + String(avgFreq, 2) + " Hz\n";
    stats += "Frequency Range: " + String(minFreq, 2) + " - " + String(maxFreq, 2) + " Hz\n";
    stats += "Avg Confidence: " + String(avgConfidence, 3) + "\n";
    stats += "Stability Rate: " + String(stabilityRate, 1) + "%\n";
    stats += "Free Space: " + String(getFreeSpace()) + " bytes";
    
    return stats;
}

size_t DataStorage::getFreeSpace() {
    return LittleFS.totalBytes() - LittleFS.usedBytes();
}

std::vector<String> DataStorage::listLogFiles() {
    std::vector<String> files;
    
    if (!_initialized) {
        return files;
    }
    
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        if (!file.isDirectory() && String(file.name()).endsWith(".csv")) {
            files.push_back(String(file.name()));
        }
        file = root.openNextFile();
    }
    
    return files;
}

int DataStorage::cleanupOldLogs(int daysToKeep) {
    if (!_initialized) {
        return 0;
    }
    
    unsigned long cutoffTime = millis() - (daysToKeep * 24 * 60 * 60 * 1000);
    int removedCount = 0;
    
    std::vector<String> files = listLogFiles();
    for (const String& filename : files) {
        if (filename == _filename) continue; // Don't remove current file
        
        File file = LittleFS.open(filename, "r");
        if (!file) continue;
        
        // Check file modification time (simplified)
        // In a real implementation, you'd store timestamps in filenames or metadata
        file.close();
        
        // For now, just remove files with "archive" in name
        if (filename.indexOf("archive") >= 0) {
            LittleFS.remove(filename);
            removedCount++;
            Logger::info("Removed old log file: " + filename);
        }
    }
    
    return removedCount;
}

String DataStorage::generateLogFilename() {
    unsigned long timestamp = millis();
    return "/data/log_" + String(timestamp) + ".csv";
}

void DataStorage::rotateLogsIfNeeded() {
    File file = LittleFS.open(_filename, "r");
    if (!file) return;
    
    size_t fileSize = file.size();
    file.close();
    
    if (fileSize > MAX_FILE_SIZE) {
        // Generate new filename
        String archiveFile = generateLogFilename();
        
        // Rename current file
        if (LittleFS.rename(_filename, archiveFile)) {
            Logger::info("Rotated log to: " + archiveFile);
            
            // Create new file with header
            File newFile = LittleFS.open(_filename, "w");
            if (newFile) {
                newFile.println("timestamp,frequency,note,octave,cents,confidence,stable,snr,crossings");
                newFile.close();
            }
        } else {
            Logger::error("Failed to rotate log file");
        }
    }
}

#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>

/**
 * @brief Estrutura para entrada de log de pitch
 * 
 * Contém todas as informações relevantes sobre uma detecção de pitch
 * para armazenamento e análise posterior.
 */
struct LogEntry {
    unsigned long timestamp;    // Timestamp em milissegundos
    float frequency;            // Frequência detectada em Hz
    String note;                // Nome da nota (A, B, C, etc.)
    int octave;                 // Oitava (0-8)
    float cents;                // Desvio em cents
    float confidence;           // Nível de confiança [0.0, 1.0]
    bool stable;                // Indicador de estabilidade
    float snr;                  // Signal-to-noise ratio
    int crossings;              // Número de zero-crossings
    
    LogEntry() : timestamp(0), frequency(0.0), note("---"), octave(0), 
                cents(0.0), confidence(0.0), stable(false), snr(0.0), crossings(0) {}
};

/**
 * @brief Sistema de armazenamento persistente de logs
 * 
 * Implementa armazenamento em LittleFS com formato CSV estruturado,
 * buffer circular em memória para logs recentes e sistema de consulta.
 */
class DataStorage {
private:
    String _filename;
    static const int BUFFER_SIZE = 100;
    LogEntry _buffer[BUFFER_SIZE];
    int _bufferIndex;
    bool _initialized;
    
    // Configurações
    static const int MAX_FILE_SIZE = 50000;  // 50KB máximo por arquivo
    static const int MAX_FILES = 5;          // Máximo de arquivos de log
    
    /**
     * @brief Gera nome de arquivo com timestamp
     * @return String com nome do arquivo
     */
    String generateLogFilename();
    
    /**
     * @brief Rotaciona arquivos de log se necessário
     */
    void rotateLogsIfNeeded();
    
    /**
     * @brief Formata entrada de log como CSV
     * @param entry Entrada de log
     * @return String formatada em CSV
     */
    String formatLogEntry(const LogEntry& entry);
    
    /**
     * @brief Parse linha CSV para LogEntry
     * @param csvLine Linha CSV
     * @return LogEntry parseada
     */
    LogEntry parseCSVLine(const String& csvLine);

public:
    /**
     * @brief Construtor
     * @param filename Nome do arquivo de log principal
     */
    DataStorage(const String& filename = "/data/logs.csv");
    
    /**
     * @brief Inicializa sistema de armazenamento
     * @return true se inicialização bem-sucedida
     */
    bool initialize();
    
    /**
     * @brief Salva entrada de log
     * @param entry Entrada de log a ser salva
     * @return true se salvamento bem-sucedido
     */
    bool save(const LogEntry& entry);
    
    /**
     * @brief Consulta logs com filtros
     * @param filter Filtro de consulta (ex: "frequency>400", "note=A")
     * @param limit Número máximo de resultados
     * @return Vetor de entradas de log
     */
    std::vector<LogEntry> query(const String& filter = "", int limit = 50);
    
    /**
     * @brief Retorna logs recentes do buffer
     * @param count Número de logs recentes
     * @return Vetor de logs recentes
     */
    std::vector<LogEntry> getRecentLogs(int count = 10);
    
    /**
     * @brief Limpa todos os logs
     * @return true se limpeza bem-sucedida
     */
    bool clear();
    
    /**
     * @brief Exporta logs para formato CSV
     * @param filename Nome do arquivo de exportação
     * @return true se exportação bem-sucedida
     */
    bool exportCSV(const String& filename);
    
    /**
     * @brief Obtém estatísticas dos logs
     * @return String com estatísticas formatadas
     */
    String getStatistics();
    
    /**
     * @brief Verifica se sistema está inicializado
     * @return true se inicializado
     */
    bool isInitialized() const { return _initialized; }
    
    /**
     * @brief Obtém espaço disponível no sistema de arquivos
     * @return Espaço disponível em bytes
     */
    size_t getFreeSpace();
    
    /**
     * @brief Lista todos os arquivos de log
     * @return Vetor com nomes dos arquivos
     */
    std::vector<String> listLogFiles();
    
    /**
     * @brief Remove arquivos de log antigos
     * @param daysToKeep Dias para manter os logs
     * @return Número de arquivos removidos
     */
    int cleanupOldLogs(int daysToKeep = 7);
};

#endif // DATA_STORAGE_H

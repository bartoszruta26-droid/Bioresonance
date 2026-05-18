package com.resonet.nano

/**
 * Baza danych częstotliwości biorezonansowych
 * Wczytywana z pliku frequencies.md
 * Format: freq_hz|category|subcategory|description|modulation|carrier_khz
 */
data class FrequencyEntry(
    val frequencyHz: Int,
    val category: String,
    val subcategory: String,
    val description: String,
    val modulation: String,
    val carrierKhz: Int
) {
    /**
     * Nazwa choroby/schorowania (przed ukośnikiem w opisie)
     */
    val diseaseName: String
        get() = description.substringBefore("/").trim()
    
    /**
     * Pełny opis z tłumaczeniem
     */
    val fullDescription: String
        get() = description
    
    /**
     * Formatowanie do wyświetlania
     */
    fun toDisplayString(): String {
        return "$diseaseName ($frequencyHz Hz)"
    }
}

/**
 * Kategorie częstotliwości zgodne z frequencies.md
 */
enum class FrequencyCategory(val displayName: String) {
    INJURY_BONE("Urazy Kości"),
    INJURY_JOINT("Stawy i Chrząstki"),
    INJURY_MUSCLE("Mięśnie"),
    INJURY_LIGAMENT("Więzadła i Ścięgna"),
    INJURY_NERVE("Nerwy"),
    INJURY_FASCIA("Powięź"),
    CIRCULATION("Krążenie"),
    ANTIPARASITIC("Antypasożytnicze"),
    MITOCHONDRIA("Mitochondria"),
    TELOMERE("Telomery"),
    DNA_REPAIR("Naprawa DNA"),
    ANTIBACTERIAL("Antybakteryjne"),
    ANTIVIRAL("Antywirusowe"),
    ANTIFUNGAL("Antygrzybicze"),
    IMMUNE("Immunologiczne"),
    HORMONAL("Hormonalne"),
    DETOX("Detoksykacja"),
    ORGAN_SPECIFIC("Organy Specyficzne"),
    PAIN("Ból"),
    NEUROLOGICAL("Neurologiczne"),
    RESPIRATORY("Oddechowe"),
    DIGESTIVE("Trawienne"),
    CARDIOVASCULAR("Kardio-waskularne"),
    LYMPHATIC("Limfatyczne"),
    SKIN("Skóra"),
    AUTOIMMUNE("Autoimmunologiczne"),
    METABOLIC("Metaboliczne"),
    SPIRITUALITY("Duchowość"),
    VIRTUES("Cnoty"),
    MERCY_ACTS("Akty Miłosierdzia"),
    CHAKRAS("Czakry"),
    HOLY_SPIRIT_GIFTS("Dary Ducha Świętego"),
    PROPHECY("Proroctwo"),
    LONGEVITY("Długowieczność");
    
    companion object {
        fun fromString(value: String): FrequencyCategory {
            return values().find { it.name == value } ?: IMMUNE
        }
    }
}

/**
 * Menadżer bazy częstotliwości
 * Singleton do ładowania i wyszukiwania częstotliwości
 */
object FrequencyDatabase {
    
    private val frequenciesById = mutableMapOf<Int, FrequencyEntry>()
    private val frequenciesByDisease = mutableMapOf<String, MutableList<FrequencyEntry>>()
    private val frequenciesByCategory = mutableMapOf<FrequencyCategory, MutableList<FrequencyEntry>>()
    private var isLoaded = false
    
    /**
     * Ładuje częstotliwości z pliku frequencies.md
     * W produkcji użyj Room/SQLite dla lepszej wydajności
     */
    fun loadFromAssets(assetsContent: String): Int {
        if (isLoaded) return frequenciesById.size
        
        val lines = assetsContent.split("\n")
        var count = 0
        
        for (line in lines) {
            // Pomijaj komentarze, nagłówki i puste linie
            if (line.startsWith("#") || line.startsWith("-") || 
                line.startsWith("=") || line.startsWith("FREQUENCY") ||
                line.startsWith("freq_hz") || line.trim().isEmpty()) {
                continue
            }
            
            // Parsuj linię: freq_hz|category|subcategory|description|modulation|carrier_khz
            val parts = line.split("|")
            if (parts.size >= 6) {
                try {
                    val freq = parts[0].toIntOrNull() ?: continue
                    val category = parts[1]
                    val subcategory = parts[2]
                    val description = parts[3]
                    val modulation = parts[4]
                    val carrier = parts[5].toIntOrNull() ?: 0
                    
                    val entry = FrequencyEntry(
                        frequencyHz = freq,
                        category = category,
                        subcategory = subcategory,
                        description = description,
                        modulation = modulation,
                        carrierKhz = carrier
                    )
                    
                    frequenciesById[freq] = entry
                    
                    // Indeksuj po nazwie choroby
                    val diseaseName = entry.diseaseName
                    frequenciesByDisease.getOrPut(diseaseName) { mutableListOf() }.add(entry)
                    
                    // Indeksuj po kategorii
                    val freqCategory = FrequencyCategory.fromString(category)
                    frequenciesByCategory.getOrPut(freqCategory) { mutableListOf() }.add(entry)
                    
                    count++
                } catch (e: Exception) {
                    // Ignoruj błędne wpisy
                }
            }
        }
        
        isLoaded = true
        return count
    }
    
    /**
     * Wyszukuje częstotliwości po nazwie choroby
     */
    fun searchByDisease(query: String): List<FrequencyEntry> {
        val results = mutableListOf<FrequencyEntry>()
        val queryLower = query.lowercase()
        
        for ((diseaseName, entries) in frequenciesByDisease) {
            if (diseaseName.lowercase().contains(queryLower)) {
                results.addAll(entries)
            }
        }
        
        return results.distinct().sortedBy { it.frequencyHz }
    }
    
    /**
     * Pobiera częstotliwości dla kategorii
     */
    fun getByCategory(category: FrequencyCategory): List<FrequencyEntry> {
        return frequenciesByCategory[category]?.sortedBy { it.frequencyHz } ?: emptyList()
    }
    
    /**
     * Pobiera wszystkie dostępne kategorie
     */
    fun getAvailableCategories(): List<FrequencyCategory> {
        return frequenciesByCategory.keys.sortedBy { it.displayName }
    }
    
    /**
     * Pobiera wpis po częstotliwości
     */
    fun getByFrequency(freq: Int): FrequencyEntry? {
        return frequenciesById[freq]
    }
    
    /**
     * Sugeruje częstotliwość na podstawie choroby
     */
    fun suggestForDisease(diseaseName: String): List<FrequencyEntry> {
        return searchByDisease(diseaseName).take(5)
    }
    
    /**
     * Czy baza jest załadowana
     */
    fun isLoaded(): Boolean = isLoaded
    
    /**
     * Liczba załadowanych częstotliwości
     */
    fun getFrequencyCount(): Int = frequenciesById.size
}

/**
 * Preset terapeutyczny - gotowa konfiguracja sesji
 */
data class TherapyPreset(
    val id: String,
    val name: String,
    val description: String,
    val frequencies: List<Int>, // Lista częstotliwości w sekwencji
    val durationPerFrequency: Int, // Czas na każdą częstotliwość w sekundach
    val totalDuration: Int, // Całkowity czas sesji
    val intensity: Int, // Intensywność 0-4095
    val modulation: ModulationType,
    val categories: List<FrequencyCategory>
) {
    companion object {
        /**
         * Tworzy preset z listy częstotliwości
         */
        fun create(
            id: String,
            name: String,
            description: String,
            frequencies: List<Int>,
            durationPerFreq: Int = 180, // 3 minuty na częstotliwość
            intensity: Int = 2048,
            modulation: ModulationType = ModulationType.NONE
        ): TherapyPreset {
            val totalDuration = frequencies.size * durationPerFreq
            val categories = frequencies.mapNotNull { freq ->
                FrequencyDatabase.getByFrequency(freq)?.let { entry ->
                    FrequencyCategory.fromString(entry.category)
                }
            }.distinct()
            
            return TherapyPreset(
                id = id,
                name = name,
                description = description,
                frequencies = frequencies,
                durationPerFrequency = durationPerFreq,
                totalDuration = totalDuration,
                intensity = intensity,
                modulation = modulation,
                categories = categories
            )
        }
    }
}

/**
 * Gotowe presety terapeutyczne
 */
object TherapyPresets {
    
    val BONE_HEALING = TherapyPreset.create(
        id = "bone_healing",
        name = "Gojenie Kości",
        description = "Terapia przyspieszająca zrosty kostne i regenerację",
        frequencies = listOf(727, 1530, 2128, 3000, 7270),
        durationPerFreq = 240,
        intensity = 2048
    )
    
    val JOINT_REPAIR = TherapyPreset.create(
        id = "joint_repair",
        name = "Regeneracja Stawów",
        description = "Terapia zwyrodnień stawowych i chrząstki",
        frequencies = listOf(666, 880, 1250, 1450, 2200, 3300),
        durationPerFreq = 180,
        intensity = 1800
    )
    
    val MUSCLE_RECOVERY = TherapyPreset.create(
        id = "muscle_recovery",
        name = "Regeneracja Mięśni",
        description = "Po treningu, urazach mięśniowych",
        frequencies = listOf(290, 320, 415, 528, 630, 740),
        durationPerFreq = 180,
        intensity = 2048,
        modulation = ModulationType.AM
    )
    
    val NERVE_PAIN = TherapyPreset.create(
        id = "nerve_pain",
        name = "Ból Nerwowy",
        description = "Neuropatie, rwa kulszowa, nerwobóle",
        frequencies = listOf(174, 285, 396, 507, 729, 840),
        durationPerFreq = 240,
        intensity = 1500,
        modulation = ModulationType.BURST
    )
    
    val DETOX_GENERAL = TherapyPreset.create(
        id = "detox_general",
        name = "Detoksykacja Ogólna",
        description = "Oczyszczanie organizmu z toksyn",
        frequencies = listOf(727, 880, 1530, 2128),
        durationPerFreq = 300,
        intensity = 2048
    )
    
    val ALL_PRESETS = listOf(
        BONE_HEALING,
        JOINT_REPAIR,
        MUSCLE_RECOVERY,
        NERVE_PAIN,
        DETOX_GENERAL
    )
    
    fun getPresetById(id: String): TherapyPreset? {
        return ALL_PRESETS.find { it.id == id }
    }
}

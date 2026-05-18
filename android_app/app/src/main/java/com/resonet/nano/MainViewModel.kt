package com.resonet.nano

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/**
 * ViewModel główny aplikacji ResoNet Nano
 * Zarządza stanem UI, połączeniem i konfiguracją efektorów
 * Rozbudowany o pełną funkcjonalność z bashtui, cpptui i webui
 */
class MainViewModel : ViewModel() {
    
    // Stan połączenia
    private val _connectionState = MutableStateFlow<ConnectionState>(ConnectionState.Disconnected)
    val connectionState: StateFlow<ConnectionState> = _connectionState.asStateFlow()
    
    // Status systemu
    private val _systemStatus = MutableStateFlow(SystemStatus())
    val systemStatus: StateFlow<SystemStatus> = _systemStatus.asStateFlow()
    
    // Lista kanałów (8 kanałów jak w webui i bashtui)
    private val _channels = MutableStateFlow<List<ChannelConfig>>(emptyList())
    val channels: StateFlow<List<ChannelConfig>> = _channels.asStateFlow()
    
    // Tryb pracy
    private val _probeMode = MutableStateFlow(ProbeMode.SINGLE)
    val probeMode: StateFlow<ProbeMode> = _probeMode.asStateFlow()
    
    // Logi systemowe
    private val _logs = MutableStateFlow<List<LogEntry>>(emptyList())
    val logs: StateFlow<List<LogEntry>> = _logs.asStateFlow()
    
    // Komunikaty dla użytkownika
    private val _message = MutableStateFlow<String?>(null)
    val message: StateFlow<String?> = _message.asStateFlow()
    
    // Czy trwa ładowanie
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()
    
    // Baza częstotliwości
    private val _frequenciesLoaded = MutableStateFlow(false)
    val frequenciesLoaded: StateFlow<Boolean> = _frequenciesLoaded.asStateFlow()
    
    // Sesja terapeutyczna
    private val _sessionState = MutableStateFlow<TherapySessionManager.SessionState>(TherapySessionManager.SessionState.Idle)
    val sessionState: StateFlow<TherapySessionManager.SessionState> = _sessionState.asStateFlow()
    
    // Interfejs komunikacyjny
    private val communication = ArduinoCommunication()
    
    // Menadżer sesji terapeutycznych
    private lateinit var therapySessionManager: TherapySessionManager
    
    // Menadżer harmonogramu
    private val scheduleManager = ScheduleManager()
    
    init {
        // Inicjalizacja domyślnych 8 kanałów
        initializeDefaultChannels()
        
        // Inicjalizacja menadżera sesji
        therapySessionManager = TherapySessionManager(communication)
        
        // Obserwacja stanu sesji
        viewModelScope.launch {
            therapySessionManager.sessionState.collect { state ->
                _sessionState.value = state
            }
        }
        
        // Nasłuchiwanie zmian połączenia
        communication.addConnectionListener(object : ArduinoCommunication.ConnectionStateListener {
            override fun onConnectionStateChanged(state: ConnectionState) {
                _connectionState.value = state
                viewModelScope.launch {
                    when (state) {
                        is ConnectionState.Connected -> {
                            showMessage("Połączono z ${state.ipAddress}:${state.port}")
                            refreshStatus()
                            refreshLogs()
                        }
                        is ConnectionState.Error -> {
                            showMessage("Błąd: ${state.message}")
                        }
                        else -> {}
                    }
                }
            }
        })
        
        // Ładowanie bazy częstotliwości
        loadFrequencyDatabase()
    }
    
    /**
     * Inicjalizuje domyślne 8 kanałów zgodnie z bash_tui
     */
    private fun initializeDefaultChannels() {
        val defaultConfigs = listOf(
            ChannelConfig(1, "Cewka Płaska", EffectorType.HELMHOLTZ, 727.0f),
            ChannelConfig(2, "Cewka Ferrytowa", EffectorType.HELMHOLTZ, 10000.0f),
            ChannelConfig(3, "Płyta Kapacytacyjna", EffectorType.CONTACT, 5000.0f),
            ChannelConfig(4, "Aplikator Punktowy", EffectorType.OTIC, 25000.0f),
            ChannelConfig(5, "Mata EMF", EffectorType.WRAP, 78.3f),
            ChannelConfig(6, "Podkładka Lokalna", EffectorType.CONTACT, 1000.0f),
            ChannelConfig(7, "Pierścień", EffectorType.OTIC, 500.0f),
            ChannelConfig(8, "Niestandardowy", EffectorType.NONE, 10.0f)
        )
        _channels.value = defaultConfigs
    }
    
    /**
     * Łączy się z urządzeniem Arduino
     */
    fun connect(ipAddress: String, port: Int = ArduinoCommunication.DEFAULT_PORT) {
        viewModelScope.launch {
            _isLoading.value = true
            val result = communication.connect(ipAddress, port)
            result.onFailure { error ->
                showMessage("Nie udało się połączyć: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Rozłącza z urządzeniem
     */
    fun disconnect() {
        viewModelScope.launch {
            communication.disconnect()
            showMessage("Rozłączono")
        }
    }
    
    /**
     * Odświeża status systemu
     */
    fun refreshStatus() {
        viewModelScope.launch {
            if (_connectionState.value !is ConnectionState.Connected) return@launch
            
            _isLoading.value = true
            val result = communication.getStatus()
            result.onSuccess { status ->
                _systemStatus.value = status
            }.onFailure { error ->
                showMessage("Błąd pobierania statusu: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Aktualizuje konfigurację kanału
     */
    fun updateChannel(channelId: Int, updates: ChannelConfig.() -> Unit) {
        val currentChannels = _channels.value.toMutableList()
        val index = currentChannels.indexOfFirst { it.id == channelId }
        
        if (index >= 0) {
            val channel = currentChannels[index]
            channel.updates()
            
            // Walidacja
            val validation = channel.validate()
            if (!validation.isValid) {
                showMessage(validation.errorMessage!!)
                return
            }
            
            currentChannels[index] = channel
            _channels.value = currentChannels
        }
    }
    
    /**
     * Wysyła konfigurację kanału do Arduino
     */
    fun sendChannelConfig(channelId: Int) {
        viewModelScope.launch {
            val channel = _channels.value.find { it.id == channelId } ?: return@launch
            
            _isLoading.value = true
            val result = communication.configureChannel(channel)
            result.onSuccess {
                showMessage("Kanał $channelId skonfigurowany")
            }.onFailure { error ->
                showMessage("Błąd konfiguracji: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Start terapii
     */
    fun startTherapy() {
        viewModelScope.launch {
            _isLoading.value = true
            val result = communication.startTherapy()
            result.onSuccess {
                showMessage("Terapia rozpoczęta")
                refreshStatus()
            }.onFailure { error ->
                showMessage("Błąd startu terapii: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Stop terapii
     */
    fun stopTherapy() {
        viewModelScope.launch {
            _isLoading.value = true
            val result = communication.stopTherapy()
            result.onSuccess {
                showMessage("Terapia zatrzymana")
                refreshStatus()
            }.onFailure { error ->
                showMessage("Błąd stopu terapii: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Skanuje podłączone efektory
     */
    fun scanDevices() {
        viewModelScope.launch {
            _isLoading.value = true
            val result = communication.scanDevices()
            result.onSuccess { effector ->
                showMessage("Wykryto: ${effector.displayName}")
                // Zaktualizuj pierwszy niewłączony kanał z wykrytym efektorem
                updateChannel(1) {
                    this.effectorType = effector
                }
            }.onFailure { error ->
                showMessage("Błąd skanowania: ${error.message}")
            }
            _isLoading.value = false
        }
    }
    
    /**
     * Pobiera logi systemowe
     */
    fun refreshLogs() {
        viewModelScope.launch {
            val result = communication.getLogs()
            result.onSuccess { entries ->
                _logs.value = entries
            }.onFailure { error ->
                showMessage("Błąd pobierania logów: ${error.message}")
            }
        }
    }
    
    /**
     * Ustawia tryb pracy
     */
    fun setProbeMode(mode: ProbeMode) {
        _probeMode.value = mode
        showMessage("Tryb: ${mode.displayName}")
    }
    
    /**
     * Włącza/wyłącza kanał
     */
    fun toggleChannel(channelId: Int) {
        updateChannel(channelId) {
            enabled = !enabled
        }
        // Wyślij konfigurację jeśli kanał został włączony
        if (_channels.value.find { it.id == channelId }?.enabled == true) {
            sendChannelConfig(channelId)
        }
    }
    
    /**
     * Pokazuje komunikat dla użytkownika
     */
    private fun showMessage(msg: String) {
        _message.value = msg
        // Automatyczne ukrycie po 3 sekundach
        viewModelScope.launch {
            kotlinx.coroutines.delay(3000)
            _message.value = null
        }
    }
    
    /**
     * Ładowanie bazy częstotliwości z pliku frequencies.md
     */
    fun loadFrequencyDatabase() {
        viewModelScope.launch {
            try {
                // W produkcji: wczytaj z assets/frequencies.md
                val mockContent = """
                    727|INJURY_BONE|general|Podstawowa regeneracja kości / Basic bone regeneration|AM|0
                    1530|INJURY_BONE|healing|Przyspieszenie zrostów kostnych / Bone union acceleration|AM|0
                    666|INJURY_JOINT|cartilage|Regeneracja chrząstki stawowej / Articular cartilage regeneration|AM|0
                    880|INJURY_JOINT|lubrication|Nawilżanie stawów / Joint lubrication|AM|0
                    290|INJURY_MUSCLE|relax|Rozluźnienie mięśni napiętych / Muscle tension relief|AM|0
                    528|INJURY_MUSCLE|fiber|Regeneracja włókien mięśniowych / Muscle fiber regeneration|FM|0
                    174|INJURY_NERVE|pain|Łagodzenie bólu nerwowego / Nerve pain relief|PWM|0
                    729|INJURY_NERVE|vagus|Stymulacja nerwu błędnego / Vagus nerve stimulation|PWM|0
                """.trimIndent()
                
                val count = FrequencyDatabase.loadFromAssets(mockContent)
                _frequenciesLoaded.value = true
                showMessage("Załadowano $count częstotliwości")
            } catch (e: Exception) {
                showMessage("Błąd ładowania częstotliwości: ${e.message}")
            }
        }
    }
    
    /**
     * Wyszukuje częstotliwości po nazwie choroby
     */
    fun searchFrequencies(query: String): List<FrequencyEntry> {
        return FrequencyDatabase.searchByDisease(query)
    }
    
    /**
     * Pobiera częstotliwości dla kategorii
     */
    fun getFrequenciesByCategory(category: FrequencyCategory): List<FrequencyEntry> {
        return FrequencyDatabase.getByCategory(category)
    }
    
    /**
     * Start sesji terapeutycznej z presetu
     */
    fun startTherapySession(preset: TherapyPreset) {
        therapySessionManager.startSession(preset)
    }
    
    /**
     * Pauza sesji
     */
    fun pauseTherapySession() {
        therapySessionManager.pauseSession()
    }
    
    /**
     * Wznowienie sesji
     */
    fun resumeTherapySession() {
        therapySessionManager.resumeSession()
    }
    
    /**
     * Stop sesji
     */
    fun stopTherapySession() {
        therapySessionManager.stopSession()
    }
    
    /**
     * Tworzy niestandardową sesję
     */
    fun createCustomTherapy(
        name: String,
        frequencies: List<Int>,
        durationPerFreq: Int = 180,
        intensity: Int = 2048
    ) {
        val preset = therapySessionManager.createCustomSession(
            name = name,
            frequencies = frequencies,
            durationPerFreq = durationPerFreq,
            intensity = intensity
        )
        startTherapySession(preset)
    }
    
    /**
     * Dodaje harmonogram terapii
     */
    fun addSchedule(
        presetId: String,
        scheduledTime: Long,
        repeatDays: List<Int> = emptyList(),
        notes: String = ""
    ) {
        val schedule = TherapySchedule(
            id = "schedule_${System.currentTimeMillis()}",
            presetId = presetId,
            scheduledTime = scheduledTime,
            repeatDays = repeatDays,
            notes = notes
        )
        scheduleManager.addSchedule(schedule)
        showMessage("Dodano harmonogram")
    }
    
    /**
     * Pobiera zaplanowane sesje na dziś
     */
    fun getScheduledToday(): List<TherapySchedule> {
        return scheduleManager.getScheduledToday()
    }
    
    /**
     * Pobiera wszystkie presety
     */
    fun getAllPresets(): List<TherapyPreset> {
        return TherapyPresets.ALL_PRESETS
    }
    
    /**
     * Dostępne kategorie częstotliwości
     */
    fun getAvailableCategories(): List<FrequencyCategory> {
        return FrequencyDatabase.getAvailableCategories()
    }

    override fun onCleared() {
        super.onCleared()
        therapySessionManager.cleanup()
        viewModelScope.launch {
            communication.disconnect()
        }
    }
}

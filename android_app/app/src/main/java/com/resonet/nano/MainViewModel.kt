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
    
    // Interfejs komunikacyjny
    private val communication = ArduinoCommunication()
    
    init {
        // Inicjalizacja domyślnych 8 kanałów
        initializeDefaultChannels()
        
        // Nasłuchiwanie zmian połączenia
        communication.addConnectionListener(object : ArduinoCommunication.ConnectionStateListener {
            override fun onConnectionStateChanged(state: ConnectionState) {
                _connectionState.value = state
                viewModelScope.launch {
                    when (state) {
                        is ConnectionState.Connected -> {
                            showMessage("Połączono z ${state.ipAddress}:${state.port}")
                            refreshStatus()
                        }
                        is ConnectionState.Error -> {
                            showMessage("Błąd: ${state.message}")
                        }
                        else -> {}
                    }
                }
            }
        })
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
    
    override fun onCleared() {
        super.onCleared()
        viewModelScope.launch {
            communication.disconnect()
        }
    }
}

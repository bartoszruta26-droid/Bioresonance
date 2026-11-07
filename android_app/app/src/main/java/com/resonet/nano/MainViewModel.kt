package com.resonet.nano

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/**
 * ViewModel klienta diagnostycznego ResoNet Protocol v1.
 *
 * Zachowuje lokalne presety kanałów wyłącznie do prezentacji. Nie udostępnia
 * operacji konfiguracji wyjść ani uruchamiania sesji, ponieważ firmware
 * obecnie nie implementuje zdalnego kontraktu sterującego.
 */
class MainViewModel : ViewModel() {

    private val _connectionState =
        MutableStateFlow<ConnectionState>(ConnectionState.Disconnected)
    val connectionState: StateFlow<ConnectionState> = _connectionState.asStateFlow()

    // Protocol v1 never proves authentication, hardware interlocks or output feedback.
    private val _deviceCapabilities =
        MutableStateFlow(DeviceCapabilities.DIAGNOSTIC_ONLY)
    val deviceCapabilities: StateFlow<DeviceCapabilities> =
        _deviceCapabilities.asStateFlow()

    private val _systemStatus = MutableStateFlow(SystemStatus())
    val systemStatus: StateFlow<SystemStatus> = _systemStatus.asStateFlow()

    /**
     * Osiem lokalnych presetów demonstracyjnych. Nie są odczytywane z urządzenia
     * i nie mogą być wysyłane jako konfiguracja wyjść.
     */
    private val _channels = MutableStateFlow<List<ChannelConfig>>(emptyList())
    val channels: StateFlow<List<ChannelConfig>> = _channels.asStateFlow()

    private val _logs = MutableStateFlow<List<LogEntry>>(emptyList())
    val logs: StateFlow<List<LogEntry>> = _logs.asStateFlow()

    private val _message = MutableStateFlow<String?>(null)
    val message: StateFlow<String?> = _message.asStateFlow()

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val communication = ArduinoCommunication()

    init {
        initializeLocalChannelPresets()

        communication.addConnectionListener(
            object : ArduinoCommunication.ConnectionStateListener {
                override fun onConnectionStateChanged(state: ConnectionState) {
                    _connectionState.value = state
                    // A v1 HELLO/capability bitfield is not a verified control contract.
                    _deviceCapabilities.value = DeviceCapabilities.DIAGNOSTIC_ONLY
                    viewModelScope.launch {
                        when (state) {
                            is ConnectionState.Connected -> {
                                showMessage("Połączono z ${state.ipAddress}:${state.port}")
                                refreshStatus()
                            }
                            is ConnectionState.Error -> showMessage("Błąd: ${state.message}")
                            is ConnectionState.Disconnected -> {
                                // Status urządzenia przestaje być aktualny po rozłączeniu.
                                _systemStatus.value = SystemStatus()
                            }
                            is ConnectionState.Connecting -> Unit
                        }
                    }
                }
            }
        )
    }

    private fun initializeLocalChannelPresets() {
        _channels.value = listOf(
            ChannelConfig(1, "Cewka Płaska", EffectorType.HELMHOLTZ, 727.0f),
            ChannelConfig(2, "Cewka Ferrytowa", EffectorType.HELMHOLTZ, 10000.0f),
            ChannelConfig(3, "Płyta Kapacytacyjna", EffectorType.CONTACT, 5000.0f),
            ChannelConfig(4, "Aplikator Punktowy", EffectorType.OTIC, 25000.0f),
            ChannelConfig(5, "Mata EMF", EffectorType.WRAP, 78.3f),
            ChannelConfig(6, "Podkładka Lokalna", EffectorType.CONTACT, 1000.0f),
            ChannelConfig(7, "Pierścień", EffectorType.OTIC, 500.0f),
            ChannelConfig(8, "Niestandardowy", EffectorType.NONE, 10.0f)
        )
    }

    fun connect(ipAddress: String, port: Int = ArduinoCommunication.DEFAULT_PORT) {
        viewModelScope.launch {
            _isLoading.value = true
            try {
                communication.connect(ipAddress, port).onFailure { error ->
                    showMessage("Nie udało się połączyć: ${error.message}")
                }
            } finally {
                _isLoading.value = false
            }
        }
    }

    fun disconnect() {
        viewModelScope.launch {
            communication.disconnect()
            _systemStatus.value = SystemStatus()
            showMessage("Rozłączono")
        }
    }

    fun refreshStatus() {
        viewModelScope.launch {
            if (_connectionState.value !is ConnectionState.Connected) return@launch

            _isLoading.value = true
            try {
                communication.getStatus()
                    .onSuccess { status -> _systemStatus.value = status }
                    .onFailure { error ->
                        showMessage("Błąd pobierania statusu: ${error.message}")
                    }
            } finally {
                _isLoading.value = false
            }
        }
    }

    private fun showMessage(message: String) {
        _message.value = message
    }

    override fun onCleared() {
        communication.close()
        super.onCleared()
    }
}

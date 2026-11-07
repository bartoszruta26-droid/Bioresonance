package com.resonet.nano

import android.os.Bundle
import android.view.View
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import com.resonet.nano.databinding.ActivityMainBinding
import kotlinx.coroutines.launch

/**
 * Główna aktywność aplikacji ResoNet Nano
 * Interfejs klienta diagnostycznego ResoNet Protocol v1
 */
class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private val viewModel: MainViewModel by viewModels()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Inicjalizacja ViewBinding
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // Konfiguracja UI
        setupUI()
        
        // Obserwacja stanu ViewModel
        observeViewModel()
    }
    
    /**
     * Konfiguruje elementy UI i listenerów
     */
    private fun setupUI() {
        // Przycisk połączenia/rozłączenia
        binding.btnConnect.setOnClickListener {
            val ip = binding.etIpAddress.text.toString().takeIf { it.isNotEmpty() } ?: "192.168.1.100"
            val port = binding.etPort.text.toString().toIntOrNull() ?: 5001
            
            when (viewModel.connectionState.value) {
                is ConnectionState.Connected -> viewModel.disconnect()
                else -> viewModel.connect(ip, port)
            }
        }
        
        // ResoNet Protocol v1 udostępnia obecnie wyłącznie diagnostykę.
        // Nie podpinaj akcji wykonawczych, których transport nie obsługuje.
        binding.btnStartTherapy.isEnabled = false
        binding.btnStopTherapy.isEnabled = false
        binding.btnScanDevices.isEnabled = false
        
        // Odśwież status
        binding.btnRefreshStatus.setOnClickListener {
            viewModel.refreshStatus()
        }
        
        // Zdalne logi nie są częścią diagnostycznego protokołu v1.
        binding.btnRefreshLogs.isEnabled = false
    }
    
    /**
     * Obserwuje zmiany w ViewModel i aktualizuje UI
     */
    private fun observeViewModel() {
        lifecycleScope.launch {
            repeatOnLifecycle(Lifecycle.State.STARTED) {
                launch {
                    viewModel.connectionState.collect { state ->
                        updateConnectionUI(state)
                    }
                }
                
                launch {
                    viewModel.systemStatus.collect { status ->
                        updateStatusUI(status)
                    }
                }
                
                launch {
                    viewModel.channels.collect { channels ->
                        updateChannelsUI(channels)
                    }
                }
                
                launch {
                    viewModel.message.collect { msg ->
                        msg?.let { showMessage(it) }
                    }
                }
                
                launch {
                    viewModel.isLoading.collect { loading ->
                        binding.progressBar.visibility = if (loading) View.VISIBLE else View.GONE
                    }
                }
                
                launch {
                    viewModel.logs.collect { entries ->
                        updateLogsUI(entries)
                    }
                }
            }
        }
    }
    
    /**
     * Aktualizuje UI stanu połączenia
     */
    private fun updateConnectionUI(state: ConnectionState) {
        when (state) {
            is ConnectionState.Disconnected -> {
                binding.tvConnectionStatus.text = "Rozłączony"
                binding.tvConnectionStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                binding.btnConnect.text = "Połącz"
                binding.etIpAddress.isEnabled = true
                binding.etPort.isEnabled = true
            }
            is ConnectionState.Connecting -> {
                binding.tvConnectionStatus.text = "Łączenie..."
                binding.tvConnectionStatus.setTextColor(getColor(android.R.color.holo_orange_dark))
                binding.btnConnect.text = "Łączenie..."
                binding.btnConnect.isEnabled = false
            }
            is ConnectionState.Connected -> {
                binding.tvConnectionStatus.text = "Połączono: ${state.ipAddress}:${state.port}"
                binding.tvConnectionStatus.setTextColor(getColor(android.R.color.holo_green_dark))
                binding.btnConnect.text = "Rozłącz"
                binding.btnConnect.isEnabled = true
                binding.etIpAddress.isEnabled = false
                binding.etPort.isEnabled = false
            }
            is ConnectionState.Error -> {
                binding.tvConnectionStatus.text = "Błąd: ${state.message}"
                binding.tvConnectionStatus.setTextColor(getColor(android.R.color.holo_red_dark))
                binding.btnConnect.text = "Połącz"
                binding.btnConnect.isEnabled = true
                binding.etIpAddress.isEnabled = true
                binding.etPort.isEnabled = true
            }
        }
    }
    
    /**
     * Aktualizuje UI statusu systemu
     */
    private fun updateStatusUI(status: SystemStatus) {
        binding.tvUptime.text = "Czas pracy: ${status.uptimeSeconds}s"
        binding.tvTemperature.text = if (status.temperatureValid)
            "Temperatura: ${status.temperatureCelsius}°C" else "Temperatura: brak wiarygodnego pomiaru"
        binding.tvMemory.text = "Wolna pamięć: ${status.freeMemoryBytes} B"
        binding.tvPwmStatus.text = if (status.pwmStatusKnown)
            "PWM: ${if (status.pwmIsActive) "AKTYWNY" else "STOP"}" else "PWM: brak danych"
        binding.tvFrequency.text = if (status.frequencyKnown)
            "Częstotliwość: ${status.currentFrequency} Hz" else "Częstotliwość: brak danych"
        binding.tvNetworkStatus.text = "Sieć: ${if (status.networkConnected) "POŁĄCZONA" else "ROZŁĄCZONA"}"
        binding.tvEffectorType.text = if (status.effectorKnown)
            "Efektor: ${status.detectedEffector.displayName}" else "Efektor: brak danych"
        
        // Kolorowanie statusu bezpieczeństwa
        val safetyColor = when {
            !status.systemStateReported -> android.R.color.holo_orange_dark
            status.hasError -> android.R.color.holo_red_dark
            status.state == SystemState.DIAGNOSTIC || status.state == SystemState.BUSY ||
                (status.diagnosticFaultFlags and 0x0002) != 0 || status.outputInhibited == null -> android.R.color.holo_orange_dark
            else -> android.R.color.holo_green_dark
        }
        binding.tvSafetyStatus.setTextColor(getColor(safetyColor))
        binding.tvSafetyStatus.text = when {
            !status.systemStateReported -> "BRAK DANYCH O STANIE"
            status.hasError -> "BŁĄD"
            status.state == SystemState.DIAGNOSTIC || status.state == SystemState.BUSY -> "TRYB DIAGNOSTYCZNY"
            (status.diagnosticFaultFlags and 0x0002) != 0 -> "OSTRZEŻENIE"
            status.outputInhibited == null -> "WYJŚCIA NIEZWERYFIKOWANE"
            status.outputInhibited == true -> "WYJŚCIA ZABLOKOWANE"
            else -> "STATUS DIAGNOSTYCZNY"
        }
    }
    
    /**
     * Aktualizuje UI listy kanałów
     * W pełnej implementacji użyj RecyclerView z adapterem
     */
    private fun updateChannelsUI(channels: List<ChannelConfig>) {
        val nameViews = listOf(
            binding.tvChannel1Name, binding.tvChannel2Name,
            binding.tvChannel3Name, binding.tvChannel4Name,
            binding.tvChannel5Name, binding.tvChannel6Name,
            binding.tvChannel7Name, binding.tvChannel8Name
        )
        val frequencyViews = listOf(
            binding.tvChannel1Freq, binding.tvChannel2Freq,
            binding.tvChannel3Freq, binding.tvChannel4Freq,
            binding.tvChannel5Freq, binding.tvChannel6Freq,
            binding.tvChannel7Freq, binding.tvChannel8Freq
        )
        val stateViews = listOf(
            binding.tvChannel1State, binding.tvChannel2State,
            binding.tvChannel3State, binding.tvChannel4State,
            binding.tvChannel5State, binding.tvChannel6State,
            binding.tvChannel7State, binding.tvChannel8State
        )

        nameViews.forEachIndexed { index, view ->
            val channel = channels.firstOrNull { it.id == index + 1 }
            view.text = channel?.name ?: "Kanał ${index + 1} — brak danych"
            frequencyViews[index].text = channel?.let { "${it.frequency} Hz" } ?: "— Hz"
            stateViews[index].text = if (channel == null) {
                "Brak danych"
            } else {
                "Preset lokalny"
            }
        }
    }
    
    /**
     * Aktualizuje UI logów
     */
    private fun updateLogsUI(entries: List<LogEntry>) {
        val logText = entries.joinToString("\n") { entry ->
            "[${entry.level.displayName}] ${entry.message}"
        }
        binding.tvLogs.text = logText.ifEmpty { "Brak logów" }
    }
    
    /**
     * Pokazuje komunikat Snackbar
     */
    private fun showMessage(message: String) {
        com.google.android.material.snackbar.Snackbar.make(
            binding.root,
            message,
            com.google.android.material.snackbar.Snackbar.LENGTH_SHORT
        ).show()
    }
    
    override fun onDestroy() {
        super.onDestroy()
        // ViewModel automatycznie rozłączy się w onCleared()
    }
}

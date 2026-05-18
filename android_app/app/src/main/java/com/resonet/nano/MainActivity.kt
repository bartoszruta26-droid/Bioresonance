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
 * Interfejs użytkownika do obsługi efektorów Arduino Nano
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
        
        // Start terapii
        binding.btnStartTherapy.setOnClickListener {
            viewModel.startTherapy()
        }
        
        // Stop terapii
        binding.btnStopTherapy.setOnClickListener {
            viewModel.stopTherapy()
        }
        
        // Skanowanie urządzeń
        binding.btnScanDevices.setOnClickListener {
            viewModel.scanDevices()
        }
        
        // Odśwież status
        binding.btnRefreshStatus.setOnClickListener {
            viewModel.refreshStatus()
        }
        
        // Odśwież logi
        binding.btnRefreshLogs.setOnClickListener {
            viewModel.refreshLogs()
        }
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
        binding.tvTemperature.text = "Temperatura: ${status.temperatureCelsius}°C"
        binding.tvMemory.text = "Wolna pamięć: ${status.freeMemoryBytes} B"
        binding.tvPwmStatus.text = "PWM: ${if (status.pwmIsActive) "AKTYWNY" else "STOP"}"
        binding.tvFrequency.text = "Częstotliwość: ${status.currentFrequency} Hz"
        binding.tvNetworkStatus.text = "Sieć: ${if (status.networkConnected) "POŁĄCZONA" else "ROZŁĄCZONA"}"
        binding.tvEffectorType.text = "Efektor: ${status.detectedEffector.displayName}"
        
        // Kolorowanie statusu bezpieczeństwa
        val safetyColor = if (status.isSafe) 
            android.R.color.holo_green_dark 
        else 
            android.R.color.holo_red_dark
        binding.tvSafetyStatus.setTextColor(getColor(safetyColor))
        binding.tvSafetyStatus.text = if (status.hasError) "BŁĄD" else "OK"
    }
    
    /**
     * Aktualizuje UI listy kanałów
     * W pełnej implementacji użyj RecyclerView z adapterem
     */
    private fun updateChannelsUI(channels: List<ChannelConfig>) {
        // Przykład: aktualizacja pierwszego kanału
        if (channels.isNotEmpty()) {
            val ch = channels[0]
            binding.tvChannel1Name.text = ch.name
            binding.tvChannel1Freq.text = "${ch.frequency} Hz"
            binding.switchChannel1.isChecked = ch.enabled
        }
        
        // W produkcji: zaimplementuj pełną listę 8 kanałów z RecyclerView
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

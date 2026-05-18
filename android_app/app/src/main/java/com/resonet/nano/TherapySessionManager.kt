package com.resonet.nano

import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

/**
 * Menadżer sesji terapeutycznych
 * Zarządza sekwencjami częstotliwości, timerami i harmonogramem terapii
 */
class TherapySessionManager {
    
    /**
     * Stan sesji terapeutycznej
     */
    sealed class SessionState {
        object Idle : SessionState()
        object Preparing : SessionState()
        data class Running(
            val currentFrequencyIndex: Int,
            val totalFrequencies: Int,
            val currentFrequency: Int,
            val timeRemainingInStep: Int,
            val totalTimeRemaining: Int
        ) : SessionState()
        object Paused : SessionState()
        data class Completed(val summary: SessionSummary) : SessionState()
        data class Error(val message: String) : SessionState()
    }
    
    /**
     * Podsumowanie zakończonej sesji
     */
    data class SessionSummary(
        val presetId: String,
        val presetName: String,
        val startTime: Long,
        val endTime: Long,
        val completedSteps: Int,
        val totalSteps: Int,
        val totalDurationSeconds: Int
    )
    
    // State flows
    private val _sessionState = MutableStateFlow<SessionState>(SessionState.Idle)
    val sessionState: StateFlow<SessionState> = _sessionState.asStateFlow()
    
    private val _isRunning = MutableStateFlow(false)
    val isRunning: StateFlow<Boolean> = _isRunning.asStateFlow()
    
    private var currentPreset: TherapyPreset? = null
    private var job: Job? = null
    private var startTime: Long = 0
    
    private val communication: ArduinoCommunication
    private val scope = CoroutineScope(Dispatchers.Main + SupervisorJob())
    
    constructor(communication: ArduinoCommunication) {
        this.communication = communication
    }
    
    /**
     * Rozpoczyna sesję terapeutyczną z presetu
     */
    fun startSession(preset: TherapyPreset) {
        if (_isRunning.value) {
            _sessionState.value = SessionState.Error("Sesja już trwa")
            return
        }
        
        currentPreset = preset
        _sessionState.value = SessionState.Preparing
        startTime = System.currentTimeMillis()
        
        job = scope.launch {
            try {
                _isRunning.value = true
                
                // Konfiguruj pierwszy kanał z pierwszą częstotliwością
                if (preset.frequencies.isNotEmpty()) {
                    val firstFreq = preset.frequencies[0]
                    val config = ChannelConfig(
                        id = 1,
                        name = preset.name,
                        frequency = firstFreq.toFloat(),
                        intensity = preset.intensity,
                        modulation = preset.modulation,
                        enabled = true
                    )
                    
                    communication.configureChannel(config)
                }
                
                // Uruchom terapię
                communication.startTherapy()
                
                // Wykonaj sekwencję częstotliwości
                runFrequencySequence(preset)
                
                // Zakończ sesję
                communication.stopTherapy()
                
                val summary = SessionSummary(
                    presetId = preset.id,
                    presetName = preset.name,
                    startTime = startTime,
                    endTime = System.currentTimeMillis(),
                    completedSteps = preset.frequencies.size,
                    totalSteps = preset.frequencies.size,
                    totalDurationSeconds = preset.totalDuration
                )
                
                _sessionState.value = SessionState.Completed(summary)
                _isRunning.value = false
                
            } catch (e: Exception) {
                _sessionState.value = SessionState.Error("Błąd sesji: ${e.message}")
                _isRunning.value = false
            }
        }
    }
    
    /**
     * Wykonuje sekwencję częstotliwości z presetu
     */
    private suspend fun runFrequencySequence(preset: TherapyPreset) {
        preset.frequencies.forEachIndexed { index, freq ->
            if (!_isRunning.value) return
            
            // Aktualizuj stan
            _sessionState.value = SessionState.Running(
                currentFrequencyIndex = index,
                totalFrequencies = preset.frequencies.size,
                currentFrequency = freq,
                timeRemainingInStep = preset.durationPerFrequency,
                totalTimeRemaining = (preset.frequencies.size - index) * preset.durationPerFrequency
            )
            
            // Konfiguruj częstotliwość
            val config = ChannelConfig(
                id = 1,
                name = "${preset.name} - Krok ${index + 1}",
                frequency = freq.toFloat(),
                intensity = preset.intensity,
                modulation = preset.modulation,
                enabled = true
            )
            
            communication.configureChannel(config)
            
            // Odliczaj czas dla tej częstotliwości
            for (remaining in preset.durationPerFrequency downTo 1) {
                if (!_isRunning.value) return
                
                delay(1000) // 1 sekunda
                
                _sessionState.value = SessionState.Running(
                    currentFrequencyIndex = index,
                    totalFrequencies = preset.frequencies.size,
                    currentFrequency = freq,
                    timeRemainingInStep = remaining,
                    totalTimeRemaining = (preset.frequencies.size - index - 1) * preset.durationPerFrequency + remaining
                )
            }
        }
    }
    
    /**
     * Pauzuje sesję
     */
    fun pauseSession() {
        if (!_isRunning.value) return
        
        _isRunning.value = false
        _sessionState.value = SessionState.Paused
        job?.cancel()
    }
    
    /**
     * Wznawia sesję
     */
    fun resumeSession() {
        if (_sessionState.value !is SessionState.Paused) return
        
        val preset = currentPreset ?: return
        startSession(preset)
    }
    
    /**
     * Zatrzymuje sesję
     */
    fun stopSession() {
        _isRunning.value = false
        job?.cancel()
        currentPreset = null
        _sessionState.value = SessionState.Idle
        
        scope.launch {
            communication.stopTherapy()
        }
    }
    
    /**
     * Czy sesja jest aktywna
     */
    fun isSessionActive(): Boolean {
        return _isRunning.value || _sessionState.value is SessionState.Paused
    }
    
    /**
     * Pobiera aktualny preset
     */
    fun getCurrentPreset(): TherapyPreset? = currentPreset
    
    /**
     * Tworzy niestandardową sesję z listy częstotliwości
     */
    fun createCustomSession(
        name: String,
        frequencies: List<Int>,
        durationPerFreq: Int = 180,
        intensity: Int = 2048,
        modulation: ModulationType = ModulationType.NONE
    ): TherapyPreset {
        return TherapyPreset.create(
            id = "custom_${System.currentTimeMillis()}",
            name = name,
            description = "Sesja niestandardowa",
            frequencies = frequencies,
            durationPerFreq = durationPerFreq,
            intensity = intensity,
            modulation = modulation
        )
    }
    
    /**
     * Dodaje częstotliwość do bieżącej sesji (w trakcie trwania)
     */
    fun addFrequencyToQueue(freq: Int) {
        // W pełnej implementacji: dodaj do kolejki当前ej sesji
        // Na razie tylko logujemy
    }
    
    /**
     * Czyszczenie zasobów
     */
    fun cleanup() {
        stopSession()
        scope.cancel()
    }
}

/**
 * Harmonogram terapii - zaplanowane sesje
 */
data class TherapySchedule(
    val id: String,
    val presetId: String,
    val scheduledTime: Long, // Timestamp
    val isEnabled: Boolean = true,
    val repeatDays: List<Int> = emptyList(), // 0=Sunday, 1=Monday, etc.
    val notes: String = ""
)

/**
 * Menadżer harmonogramu terapii
 */
class ScheduleManager {
    
    private val schedules = mutableListOf<TherapySchedule>()
    
    fun addSchedule(schedule: TherapySchedule) {
        schedules.add(schedule)
    }
    
    fun removeSchedule(id: String) {
        schedules.removeAll { it.id == id }
    }
    
    fun getScheduledToday(): List<TherapySchedule> {
        val calendar = java.util.Calendar.getInstance()
        val today = calendar.get(java.util.Calendar.DAY_OF_WEEK) - 1 // 0=Sunday
        
        return schedules.filter { 
            it.isEnabled && 
            (it.repeatDays.isEmpty() || it.repeatDays.contains(today)) &&
            it.scheduledTime > System.currentTimeMillis()
        }
    }
    
    fun getAllSchedules(): List<TherapySchedule> {
        return schedules.sortedBy { it.scheduledTime }
    }
    
    fun enableSchedule(id: String, enabled: Boolean) {
        schedules.find { it.id == id }?.let { schedule ->
            schedules[schedules.indexOf(schedule)] = schedule.copy(isEnabled = enabled)
        }
    }
}

// ResoNet-Nano: Schemat połączeń Arduino Nano z efektorami
// Format: TSCircuit (https://tscircuit.com/)
// Autor: Generated for ResoNet-Nano project

import { createCircuit } from "@tscircuit/core"

const circuit = createCircuit()

circuit.add(
  // Arduino Nano - główny mikrokontroler
  {
    type: "component",
    name: "ARDUINO1",
    footprint: "arduino_nano",
    pinLabels: {
      D5: "IR_PWM",
      D9: "PWM_OUT",
      D10: "SPI_CS",
      D11: "SPI_MOSI",
      D12: "SPI_MISO",
      D13: "SPI_SCK",
      "5V": "5V",
      GND: "DGND",
      "3.3V": "3.3V_LDO"
    }
  },
  
  // Moduł Ethernet ENC28J60
  {
    type: "component",
    name: "ETH1",
    footprint: "enc28j60_module",
    pinLabels: {
      CS: "Pin 2",
      MOSI: "Pin 5",
      MISO: "Pin 6", 
      SCK: "Pin 4",
      VREG: "Pin 8",
      GND: "Pin 3"
    }
  },
  
  // Optoizolator PWM (6N137)
  {
    type: "component",
    name: "OPTO_PWM1",
    footprint: "dip8",
    pinLabels: {
      "1": "ANODE",
      "2": "CATHODE",
      "4": "EMITTER",
      "5": "COLLECTOR",
      "8": "VCC"
    }
  },
  
  // Optoizolator IR (6N137)
  {
    type: "component",
    name: "OPTO_IR1",
    footprint: "dip8",
    pinLabels: {
      "1": "ANODE",
      "2": "CATHODE",
      "4": "EMITTER",
      "5": "COLLECTOR",
      "8": "VCC"
    }
  },
  
  // Izolator DC-DC dla PWM
  {
    type: "component",
    name: "DCDC_PWM1",
    footprint: "sip7",
    pinLabels: {
      "1": "5V_IN",
      "2": "GND_IN",
      "3": "5V_ISO",
      "4": "AGND"
    }
  },
  
  // Izolator DC-DC dla IR
  {
    type: "component",
    name: "DCDC_IR1",
    footprint: "sip7",
    pinLabels: {
      "1": "5V_IN",
      "2": "GND_IN",
      "3": "5V_ISO_IR",
      "4": "GND_ISO_IR"
    }
  },
  
  // MOSFET Driver PWM (IRF540N)
  {
    type: "component",
    name: "MOSFET_PWM1",
    footprint: "to220",
    pinLabels: {
      G: "Gate",
      D: "Drain",
      S: "Source"
    }
  },
  
  // MOSFET Driver IR (IRLZ44N)
  {
    type: "component",
    name: "MOSFET_IR1",
    footprint: "to220",
    pinLabels: {
      G: "Gate",
      D: "Drain",
      S: "Source"
    }
  },
  
  // Filtr LC wyjścia PWM
  {
    type: "component",
    name: "L1",
    footprint: "inductor_radial",
    value: "100µH"
  },
  {
    type: "component",
    name: "C1",
    footprint: "capacitor_radial",
    value: "100nF MKP"
  },
  
  // Złącze BNC wyjściowe
  {
    type: "component",
    name: "BNC1",
    footprint: "bnc_panel_mount",
    pinLabels: {
      SIG: "Signal",
      GND: "Shield"
    }
  },
  
  // Pasek LED IR
  {
    type: "component",
    name: "IR_STRIP1",
    footprint: "connector_2pin",
    pinLabels: {
      "+": "VCC",
      "-": "GND"
    }
  },
  
  // Rezystory
  {
    type: "component",
    name: "R1",
    footprint: "resistor_axial",
    value: "220Ω",
    description: "Current limit PWM opto"
  },
  {
    type: "component",
    name: "R2",
    footprint: "resistor_axial",
    value: "220Ω",
    description: "Current limit IR opto"
  },
  {
    type: "component",
    name: "R3",
    footprint: "resistor_axial",
    value: "10kΩ",
    description: "Pull-up PWM opto VCC"
  },
  {
    type: "component",
    name: "R4",
    footprint: "resistor_axial",
    value: "10kΩ",
    description: "Pull-up IR opto VCC"
  },
  {
    type: "component",
    name: "R5",
    footprint: "resistor_axial",
    value: "10kΩ",
    description: "Gate pulldown PWM"
  },
  {
    type: "component",
    name: "R6",
    footprint: "resistor_axial",
    value: "10kΩ",
    description: "Gate pulldown IR"
  },
  
  // LDO 3.3V dla Ethernet
  {
    type: "component",
    name: "LDO1",
    footprint: "sot23-3",
    pinLabels: {
      "1": "VIN",
      "2": "GND",
      "3": "VOUT"
    },
    value: "MCP1700-3.3"
  },
  
  // Zasilanie wejściowe
  {
    type: "component",
    name: "PWR_IN1",
    footprint: "barrel_jack",
    pinLabels: {
      "+": "5V_IN",
      "-": "GND_IN"
    }
  }
)

// Połączenia - Ścieżka PWM z izolacją
circuit.connect("ARDUINO1.D9", "R1.1")  // D9 PWM OUT -> 220Ω
circuit.connect("R1.2", "OPTO_PWM1.1")  // 220Ω -> Anoda opto PWM
circuit.connect("ARDUINO1.GND", "OPTO_PWM1.2")  // GND -> Katoda opto PWM

circuit.connect("DCDC_PWM1.3", "R3.1")  // 5V_ISO -> Pull-up 10k
circuit.connect("R3.2", "OPTO_PWM1.8")  // Pull-up -> VCC opto
circuit.connect("DCDC_PWM1.3", "OPTO_PWM1.8")  // 5V_ISO -> VCC opto

circuit.connect("OPTO_PWM1.4", "DCDC_PWM1.4")  // Emitter -> AGND
circuit.connect("OPTO_PWM1.5", "MOSFET_PWM1.G")  // Collector -> Gate MOSFET
circuit.connect("MOSFET_PWM1.G", "R5.1")  // Gate -> Pulldown 10k
circuit.connect("R5.2", "DCDC_PWM1.4")  // Pulldown -> AGND

circuit.connect("MOSFET_PWM1.S", "DCDC_PWM1.4")  // Source -> AGND
circuit.connect("MOSFET_PWM1.D", "L1.1")  // Drain -> Cewka LC

circuit.connect("L1.2", "C1.1")  // Cewka -> Kondensator
circuit.connect("C1.2", "DCDC_PWM1.4")  // Kondensator -> AGND
circuit.connect("L1.2", "BNC1.SIG")  // Wyjście filtra -> BNC signal

circuit.connect("DCDC_PWM1.4", "BNC1.GND")  // AGND -> BNC shield

// Połączenia - Ścieżka IR z izolacją
circuit.connect("ARDUINO1.D5", "R2.1")  // D5 IR PWM -> 220Ω
circuit.connect("R2.2", "OPTO_IR1.1")  // 220Ω -> Anoda opto IR
circuit.connect("ARDUINO1.GND", "OPTO_IR1.2")  // GND -> Katoda opto IR

circuit.connect("DCDC_IR1.3", "R4.1")  // 5V_ISO_IR -> Pull-up 10k
circuit.connect("R4.2", "OPTO_IR1.8")  // Pull-up -> VCC opto
circuit.connect("DCDC_IR1.3", "OPTO_IR1.8")  // 5V_ISO_IR -> VCC opto

circuit.connect("OPTO_IR1.4", "DCDC_IR1.4")  // Emitter -> GND_ISO_IR
circuit.connect("OPTO_IR1.5", "MOSFET_IR1.G")  // Collector -> Gate MOSFET
circuit.connect("MOSFET_IR1.G", "R6.1")  // Gate -> Pulldown 10k
circuit.connect("R6.2", "DCDC_IR1.4")  // Pulldown -> GND_ISO_IR

circuit.connect("MOSFET_IR1.S", "DCDC_IR1.4")  // Source -> GND_ISO_IR
circuit.connect("MOSFET_IR1.D", "IR_STRIP1.-")  // Drain -> LED Strip GND

circuit.connect("DCDC_IR1.3", "IR_STRIP1.+")  // 5V_ISO_IR -> LED Strip VCC

// Połączenia - Ethernet SPI
circuit.connect("ARDUINO1.D10", "ETH1.CS")  // D10 -> Chip Select
circuit.connect("ARDUINO1.D11", "ETH1.MOSI")  // D11 -> MOSI
circuit.connect("ARDUINO1.D12", "ETH1.MISO")  // D12 -> MISO
circuit.connect("ARDUINO1.D13", "ETH1.SCK")  // D13 -> SCK

// LDO 3.3V dla Ethernet
circuit.connect("ARDUINO1.5V", "LDO1.VIN")  // 5V -> LDO input
circuit.connect("ARDUINO1.GND", "LDO1.GND")  // GND -> LDO GND
circuit.connect("LDO1.VOUT", "ETH1.VREG")  // 3.3V -> Ethernet VREG
circuit.connect("ARDUINO1.GND", "ETH1.GND")  // DGND -> Ethernet GND

// Zasilanie główne
circuit.connect("PWR_IN1.+", "ARDUINO1.5V")  // 5V_IN -> Arduino 5V
circuit.connect("PWR_IN1.-", "ARDUINO1.GND")  // GND_IN -> Arduino GND
circuit.connect("PWR_IN1.+", "DCDC_PWM1.1")  // 5V_IN -> DCDC PWM input
circuit.connect("PWR_IN1.-", "DCDC_PWM1.2")  // GND_IN -> DCDC PWM GND
circuit.connect("PWR_IN1.+", "DCDC_IR1.1")  // 5V_IN -> DCDC IR input
circuit.connect("PWR_IN1.-", "DCDC_IR1.2")  // GND_IN -> DCDC IR GND

export default circuit

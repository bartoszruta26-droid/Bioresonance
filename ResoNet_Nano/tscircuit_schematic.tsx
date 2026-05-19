// ResoNet-Nano: Schemat połączeń Arduino Nano z efektorami
// Format: TSCircuit (https://tscircuit.com/)
// Autor: Generated for ResoNet-Nano project

import { Board } from "@tscircuit/core"

const mainCircuit = (
  <Board>
    {/* Arduino Nano - główny mikrokontroler */}
    <component
      name="ARDUINO1"
      footprint="arduino_nano"
      pinLabels={{
        D5: "IR_PWM",
        D9: "PWM_OUT",
        D10: "SPI_CS",
        D11: "SPI_MOSI",
        D12: "SPI_MISO",
        D13: "SPI_SCK",
        "5V": "5V",
        GND: "DGND",
        "3.3V": "3.3V_LDO"
      }}
    />
    
    {/* Moduł Ethernet ENC28J60 */}
    <component
      name="ETH1"
      footprint="enc28j60_module"
      pinLabels={{
        CS: "Pin 2",
        MOSI: "Pin 5",
        MISO: "Pin 6", 
        SCK: "Pin 4",
        VREG: "Pin 8",
        GND: "Pin 3"
      }}
    />
    
    {/* Optoizolator PWM (6N137) */}
    <component
      name="OPTO_PWM1"
      footprint="dip8"
      pinLabels={{
        "1": "ANODE",
        "2": "CATHODE",
        "4": "EMITTER",
        "5": "COLLECTOR",
        "8": "VCC"
      }}
    />
    
    {/* Optoizolator IR (6N137) */}
    <component
      name="OPTO_IR1"
      footprint="dip8"
      pinLabels={{
        "1": "ANODE",
        "2": "CATHODE",
        "4": "EMITTER",
        "5": "COLLECTOR",
        "8": "VCC"
      }}
    />
    
    {/* Izolator DC-DC dla PWM */}
    <component
      name="DCDC_PWM1"
      footprint="sip7"
      pinLabels={{
        "1": "5V_IN",
        "2": "GND_IN",
        "3": "5V_ISO",
        "4": "AGND"
      }}
    />
    
    {/* Izolator DC-DC dla IR */}
    <component
      name="DCDC_IR1"
      footprint="sip7"
      pinLabels={{
        "1": "5V_IN",
        "2": "GND_IN",
        "3": "5V_ISO_IR",
        "4": "GND_ISO_IR"
      }}
    />
    
    {/* MOSFET Driver PWM (IRF540N) */}
    <component
      name="MOSFET_PWM1"
      footprint="to220"
      pinLabels={{
        G: "Gate",
        D: "Drain",
        S: "Source"
      }}
    />
    
    {/* MOSFET Driver IR (IRLZ44N) */}
    <component
      name="MOSFET_IR1"
      footprint="to220"
      pinLabels={{
        G: "Gate",
        D: "Drain",
        S: "Source"
      }}
    />
    
    {/* Filtr LC wyjścia PWM */}
    <component
      name="L1"
      footprint="inductor_radial"
      value="100µH"
    />
    <component
      name="C1"
      footprint="capacitor_radial"
      value="100nF MKP"
    />
    
    {/* Złącze BNC wyjściowe */}
    <component
      name="BNC1"
      footprint="bnc_panel_mount"
      pinLabels={{
        SIG: "Signal",
        GND: "Shield"
      }}
    />
    
    {/* Pasek LED IR */}
    <component
      name="IR_STRIP1"
      footprint="connector_2pin"
      pinLabels={{
        "+": "VCC",
        "-": "GND"
      }}
    />
    
    {/* Rezystory */}
    <component
      name="R1"
      footprint="resistor_axial"
      value="220Ω"
      description="Current limit PWM opto"
    />
    <component
      name="R2"
      footprint="resistor_axial"
      value="220Ω"
      description="Current limit IR opto"
    />
    <component
      name="R3"
      footprint="resistor_axial"
      value="10kΩ"
      description="Pull-up PWM opto VCC"
    />
    <component
      name="R4"
      footprint="resistor_axial"
      value="10kΩ"
      description="Pull-up IR opto VCC"
    />
    <component
      name="R5"
      footprint="resistor_axial"
      value="10kΩ"
      description="Gate pulldown PWM"
    />
    <component
      name="R6"
      footprint="resistor_axial"
      value="10kΩ"
      description="Gate pulldown IR"
    />
    
    {/* LDO 3.3V dla Ethernet */}
    <component
      name="LDO1"
      footprint="sot23-3"
      pinLabels={{
        "1": "VIN",
        "2": "GND",
        "3": "VOUT"
      }}
      value="MCP1700-3.3"
    />
    
    {/* Zasilanie wejściowe */}
    <component
      name="PWR_IN1"
      footprint="barrel_jack"
      pinLabels={{
        "+": "5V_IN",
        "-": "GND_IN"
      }}
    />
    
    {/* Połączenia - Ścieżka PWM z izolacją */}
    <trace from=".ARDUINO1 > .D9" to=".R1 > .pin1" />
    <trace from=".R1 > .pin2" to=".OPTO_PWM1 > .pin1" />
    <trace from=".ARDUINO1 > .GND" to=".OPTO_PWM1 > .pin2" />
    
    <trace from=".DCDC_PWM1 > .pin3" to=".R3 > .pin1" />
    <trace from=".R3 > .pin2" to=".OPTO_PWM1 > .pin8" />
    <trace from=".DCDC_PWM1 > .pin3" to=".OPTO_PWM1 > .pin8" />
    
    <trace from=".OPTO_PWM1 > .pin4" to=".DCDC_PWM1 > .pin4" />
    <trace from=".OPTO_PWM1 > .pin5" to=".MOSFET_PWM1 > .G" />
    <trace from=".MOSFET_PWM1 > .G" to=".R5 > .pin1" />
    <trace from=".R5 > .pin2" to=".DCDC_PWM1 > .pin4" />
    
    <trace from=".MOSFET_PWM1 > .S" to=".DCDC_PWM1 > .pin4" />
    <trace from=".MOSFET_PWM1 > .D" to=".L1 > .pin1" />
    
    <trace from=".L1 > .pin2" to=".C1 > .pin1" />
    <trace from=".C1 > .pin2" to=".DCDC_PWM1 > .pin4" />
    <trace from=".L1 > .pin2" to=".BNC1 > .SIG" />
    
    <trace from=".DCDC_PWM1 > .pin4" to=".BNC1 > .GND" />
    
    {/* Połączenia - Ścieżka IR z izolacją */}
    <trace from=".ARDUINO1 > .D5" to=".R2 > .pin1" />
    <trace from=".R2 > .pin2" to=".OPTO_IR1 > .pin1" />
    <trace from=".ARDUINO1 > .GND" to=".OPTO_IR1 > .pin2" />
    
    <trace from=".DCDC_IR1 > .pin3" to=".R4 > .pin1" />
    <trace from=".R4 > .pin2" to=".OPTO_IR1 > .pin8" />
    <trace from=".DCDC_IR1 > .pin3" to=".OPTO_IR1 > .pin8" />
    
    <trace from=".OPTO_IR1 > .pin4" to=".DCDC_IR1 > .pin4" />
    <trace from=".OPTO_IR1 > .pin5" to=".MOSFET_IR1 > .G" />
    <trace from=".MOSFET_IR1 > .G" to=".R6 > .pin1" />
    <trace from=".R6 > .pin2" to=".DCDC_IR1 > .pin4" />
    
    <trace from=".MOSFET_IR1 > .S" to=".DCDC_IR1 > .pin4" />
    <trace from=".MOSFET_IR1 > .D" to=".IR_STRIP1 > .-" />
    
    <trace from=".DCDC_IR1 > .pin3" to=".IR_STRIP1 > .+" />
    
    {/* Połączenia - Ethernet SPI */}
    <trace from=".ARDUINO1 > .D10" to=".ETH1 > .CS" />
    <trace from=".ARDUINO1 > .D11" to=".ETH1 > .MOSI" />
    <trace from=".ARDUINO1 > .D12" to=".ETH1 > .MISO" />
    <trace from=".ARDUINO1 > .D13" to=".ETH1 > .SCK" />
    
    {/* LDO 3.3V dla Ethernet */}
    <trace from=".ARDUINO1 > .5V" to=".LDO1 > .VIN" />
    <trace from=".ARDUINO1 > .GND" to=".LDO1 > .GND" />
    <trace from=".LDO1 > .VOUT" to=".ETH1 > .VREG" />
    <trace from=".ARDUINO1 > .GND" to=".ETH1 > .GND" />
    
    {/* Zasilanie główne */}
    <trace from=".PWR_IN1 > .+" to=".ARDUINO1 > .5V" />
    <trace from=".PWR_IN1 > .-" to=".ARDUINO1 > .GND" />
    <trace from=".PWR_IN1 > .+" to=".DCDC_PWM1 > .pin1" />
    <trace from=".PWR_IN1 > .-" to=".DCDC_PWM1 > .pin2" />
    <trace from=".PWR_IN1 > .+" to=".DCDC_IR1 > .pin1" />
    <trace from=".PWR_IN1 > .-" to=".DCDC_IR1 > .pin2" />
  </Board>
)

export default mainCircuit

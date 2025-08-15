//
//  FMCControlView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 15/08/2025.
//

import SwiftUI

struct FMCControlView: View {
    let device: WinwingDevice
    @State private var backlight: Double = 0
    @State private var screenBacklight: Double = 0
    @State private var overallLedsBrightness: Double = 0
    
    // LED states for FMC indicators
    @State private var ledStates: [Bool] = Array(repeating: false, count: 17)
    @State private var selectedTestPage: String = "INIT";
    
    private let indicatorLEDs: [(id: Int, name: String)] = [
        (8, "FAIL"), (9, "FM"), (10, "MCDU"), (11, "MENU"), 
        (12, "FM1"), (13, "IND"), (14, "RDY"), (15, "STATUS"), (16, "FM2")
    ]
    
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("FMC Controls")
                .font(.headline)
                .padding(.top, 8)
            
            // Backlight Controls
            VStack(alignment: .leading, spacing: 12) {
                Text("Backlights")
                    .font(.subheadline)
                    .fontWeight(.medium)
                
                HStack(alignment: .center, spacing: 16) {
                    Text("Backlight")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $backlight, in: 0...255, step: 1)
                        .frame(width: 160)
                    Text("\(Int(backlight))")
                        .frame(width: 36, alignment: .trailing)
                    Button(action: { setBacklight() }) {
                        Text("Set")
                    }
                    .buttonStyle(.bordered)
                }
                
                HStack(alignment: .center, spacing: 16) {
                    Text("Screen Backlight")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $screenBacklight, in: 0...255, step: 1)
                        .frame(width: 160)
                    Text("\(Int(screenBacklight))")
                        .frame(width: 36, alignment: .trailing)
                    Button(action: { setScreenBacklight() }) {
                        Text("Set")
                    }
                    .buttonStyle(.bordered)
                }
                
                HStack(alignment: .center, spacing: 16) {
                    Text("Overall LEDs")
                        .frame(width: 100, alignment: .leading)
                    Slider(value: $overallLedsBrightness, in: 0...255, step: 1)
                        .frame(width: 160)
                    Text("\(Int(overallLedsBrightness))")
                        .frame(width: 36, alignment: .trailing)
                    Button(action: { setOverallLedsBrightness() }) {
                        Text("Set")
                    }
                    .buttonStyle(.bordered)
                }
            }
            
            Divider()
                .padding(.vertical, 8)
            
            // LED Controls
            VStack(alignment: .leading, spacing: 12) {
                Text("Indicator LEDs")
                    .font(.subheadline)
                    .fontWeight(.medium)
                
                LazyVGrid(columns: Array(repeating: GridItem(.flexible()), count: 3), spacing: 8) {
                    ForEach(indicatorLEDs, id: \.id) { led in
                        Toggle(led.name, isOn: Binding(
                            get: { ledStates[led.id] },
                            set: { value in
                                ledStates[led.id] = value
                                setLED(led.id, state: value)
                            }
                        ))
                        .toggleStyle(.switch)
                        .controlSize(.mini)
                    }
                }
                
                HStack {
                    Button("All On") {
                        for led in indicatorLEDs {
                            ledStates[led.id] = true
                            setLED(led.id, state: true)
                        }
                    }
                    .buttonStyle(.bordered)
                    
                    Button("All Off") {
                        for led in indicatorLEDs {
                            ledStates[led.id] = false
                            setLED(led.id, state: false)
                        }
                    }
                    .buttonStyle(.bordered)
                    
                    Spacer()
                }
                .padding(.top, 8)
            }
            
            Divider()
                .padding(.vertical, 8)
            
            // Display Controls
            VStack(alignment: .leading, spacing: 12) {
                Text("Display Controls")
                    .font(.subheadline)
                    .fontWeight(.medium)
                
                HStack {
                    Button("Clear Display") {
                        clearDisplay()
                    }
                    .buttonStyle(.bordered)
                    
                    Spacer()
                }
                
                // Test page selection
                VStack(alignment: .leading, spacing: 8) {
                    Text("Test Pages")
                        .font(.caption)
                        .foregroundColor(.secondary)
                    
                    Picker("Test Page", selection: $selectedTestPage) {
                        Text("INIT").tag("INIT")
                        Text("MENU").tag("MENU")
                        Text("PERF").tag("PERF")
                        Text("PROG").tag("PROG")
                        Text("NAV").tag("NAV")
                        Text("FPLN").tag("FPLN")
                        Text("RAD NAV").tag("RAD NAV")
                        Text("FUEL PRED").tag("FUEL PRED")
                        Text("SEC FPLN").tag("SEC FPLN")
                        Text("ATC COMM").tag("ATC COMM")
                        Text("MCDU REPORT").tag("MCDU REPORT")
                        Text("AIDS").tag("AIDS")
                        Text("CFDS").tag("CFDS")
                    }
                    .pickerStyle(.menu)
                    .frame(width: 200)
                    
                    Button("Show Test Page") {
                        showTestPage()
                    }
                    .buttonStyle(.bordered)
                }
            }
            
            Spacer()
        }
        .padding()
        .onAppear {
            device.update()
        }
    }
    
    // MARK: - Control Functions
    
    private func setBacklight() {
        guard let fmc = device.fmc else { return }
        fmc.setBacklight(UInt8(backlight))
    }
    
    private func setScreenBacklight() {
        guard let fmc = device.fmc else { return }
        fmc.setScreenBacklight(UInt8(screenBacklight))
    }
    
    private func setOverallLedsBrightness() {
        guard let fmc = device.fmc else { return }
        fmc.setOverallLedsBrightness(UInt8(overallLedsBrightness))
    }
    
    private func setLED(_ ledId: Int, state: Bool) {
        guard let fmc = device.fmc else { return }
        fmc.setLed(ledId, state: state)
    }
    
    private func clearDisplay() {
        guard let fmc = device.fmc else { return }
        fmc.clearDisplay()
    }
    
    private func showTestPage() {
        guard let fmc = device.fmc else { return }
        
        // Map test page names to display variants
        let pageVariants: [String: Int] = [
            "INIT": 0,
            "MENU": 1,
            "PERF": 2,
            "PROG": 3,
            "NAV": 4,
            "FPLN": 5,
            "RAD NAV": 6,
            "FUEL PRED": 7,
            "SEC FPLN": 8,
            "ATC COMM": 9,
            "MCDU REPORT": 10,
            "AIDS": 11,
            "CFDS": 12
        ]
        
        if let variant = pageVariants[selectedTestPage] {
            fmc.showBackground(variant)
        }
    }
}

#Preview {
    // Create a mock device for preview
    let mockDevice = WinwingDevice(
        id: 0,
        name: "Mock FMC",
        type: .fmc,
        productId: 0x1234,
        isConnected: true
    )
    
    FMCControlView(device: mockDevice)
}

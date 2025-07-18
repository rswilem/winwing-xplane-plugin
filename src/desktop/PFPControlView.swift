//
//  PFPControlView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 09/07/2025.
//

import SwiftUI

struct PFPControlView: View {
    let device: WinwingDevice
    @State private var backlight: Double = 0
    @State private var screenBacklight: Double = 0
    
    // LED states for PFP indicators
    @State private var ledStates: [Bool] = Array(repeating: false, count: 17)
    @State private var selectedTestPage: String = "MENU"
    
    private let indicatorLEDs: [(id: Int, name: String)] = [
        (8, "FAIL"), (9, "FM"), (10, "MCDU"), (11, "MENU"), 
        (12, "FM1"), (13, "IND"), (14, "RDY"), (15, "STATUS"), (16, "FM2")
    ]
    
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("PFP Controls")
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
                    Text("Screen")
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
            }
            
            // Display Controls
            VStack(alignment: .leading, spacing: 12) {
                Text("Display Controls")
                    .font(.subheadline)
                    .fontWeight(.medium)
                
                HStack(spacing: 8) {
                    ForEach(1...8, id: \.self) { displayId in
                        Button(action: { clearDisplay(displayId) }) {
                            Text("\(displayId): \(displayId == 8 ? "Logo" : "Clear")")
                        }
                        .buttonStyle(.bordered)
                    }
                }
                
                HStack(spacing: 12) {
                    Picker("Test Page", selection: $selectedTestPage) {
                        Text("MENU").tag("MENU")
                        Text("IDENT").tag("IDENT")
                        Text("ACARS").tag("ACARS")
                        Text("TAKEOFF_REF").tag("TAKEOFF_REF")
                        Text("FLTPLN_REQ").tag("FLTPLN_REQ")
                    }
                    .pickerStyle(MenuPickerStyle())
                    
                    Button(action: { testDisplay(page: selectedTestPage) }) {
                        Text("Test Display")
                    }
                    .buttonStyle(.borderedProminent)
                    
                    Spacer()
                }
            }
            
            // LED Indicators
            VStack(alignment: .leading, spacing: 12) {
                Text("LED Indicators")
                    .font(.subheadline)
                    .fontWeight(.medium)
                
                LazyVGrid(columns: Array(repeating: GridItem(.flexible()), count: 3), spacing: 8) {
                    ForEach(indicatorLEDs, id: \.id) { led in
                        Toggle(led.name, isOn: Binding(
                            get: { ledStates[led.id] },
                            set: { newValue in
                                ledStates[led.id] = newValue
                                setLed(led.id, newValue)
                            }
                        ))
                        .toggleStyle(.switch)
                    }
                }
            }
            
            Spacer()
        }
        .padding(.vertical, 8)
        .frame(maxWidth: .infinity, alignment: .topLeading)
    }
    
    private func setBacklight() {
        guard let pfp = device.pfp else { return }
        pfp.setBacklight(UInt8(backlight))
    }
    
    private func setScreenBacklight() {
        guard let pfp = device.pfp else { return }
        pfp.setScreenBacklight(UInt8(screenBacklight))
    }
    
    private func clearDisplay(_ displayId: Int) {
        guard let pfp = device.pfp else { return }
        pfp.clear2(displayId)
    }
    
    private func testDisplay(page: String) {
        clearDatarefCache()
        
        switch page {
        case "MENU":
            setDatarefHex("laminar/B738/fmc1/Line00_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4D, 0x45, 0x4E, 0x55, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line01_L", [0x3C, 0x46, 0x4D, 0x43, 0x20, 0x20, 0x20, 0x20, 0x3C, 0x41, 0x43, 0x54, 0x3E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line02_L", [0x3C, 0x44, 0x4C, 0x4E, 0x4B, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line03_L", [0x3C, 0x41, 0x43, 0x4D, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            
            setDatarefHex("laminar/B738/fmc1/Line_entry", [0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x20, 0x57, 0x4F, 0x52, 0x4C, 0x44])
            break
            
        case "ACARS":
            setDatarefHex("laminar/B738/fmc1/Line00_L", [0x41, 0x43, 0x41, 0x52, 0x53, 0x2D, 0x53, 0x41, 0x20, 0x30, 0x39, 0x2F, 0x31, 0x32, 0x3A, 0x35, 0x35])
            setDatarefHex("laminar/B738/fmc1/Line01_X", [0x20, 0x31, 0x33, 0x3A, 0x32, 0x34, 0x20, 0x20, 0x20, 0x56, 0x49, 0x45, 0x57, 0x45, 0x44, 0x20, 0x20, 0x20, 0x30, 0x31, 0x2F, 0x30, 0x31])
            setDatarefHex("laminar/B738/fmc1/Line02_L", [0x45, 0x48, 0x41, 0x4D, 0x20, 0x30, 0x39, 0x31, 0x32, 0x35, 0x35, 0x5A, 0x20, 0x33, 0x33, 0x30, 0x30, 0x39, 0x4B, 0x54, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line02_LX", [0x53, 0x41, 0x20, 0x30, 0x39, 0x2F, 0x31, 0x32, 0x3A, 0x35, 0x35])
            setDatarefHex("laminar/B738/fmc1/Line03_L", [0x32, 0x32, 0x2F, 0x30, 0x38, 0x20, 0x51, 0x31, 0x30, 0x32, 0x31, 0x20, 0x4E, 0x4F, 0x53, 0x49, 0x47, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line03_LX", [0x32, 0x37, 0x30, 0x56, 0x30, 0x32, 0x30, 0x20, 0x39, 0x39, 0x39, 0x39, 0x20, 0x46, 0x45, 0x57, 0x30, 0x34, 0x35, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line05_L", [0x3D, 0x50, 0x52, 0x49, 0x4E, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line06_L", [0x3C, 0x52, 0x45, 0x54, 0x55, 0x52, 0x4E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line06_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x33, 0x3A, 0x32, 0x34])
            
            setDatarefHex("laminar/B738/fmc1/Line_entry_I", [0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x20, 0x57, 0x4F, 0x52, 0x4C, 0x44])
            break


        case "TAKEOFF_REF":
            setDatarefHex("laminar/B738/fmc1/Line00_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x41, 0x4B, 0x45, 0x4F, 0x46, 0x46, 0x20, 0x52, 0x45, 0x46, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line00_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x2F, 0x32, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line01_L", [0x20, 0x35, 0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x30, 0x38])
            setDatarefHex("laminar/B738/fmc1/Line01_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2D, 0x2D, 0x2D])
            setDatarefHex("laminar/B738/fmc1/Line01_X", [0x20, 0x46, 0x4C, 0x41, 0x50, 0x53, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x51, 0x52, 0x48, 0x20, 0x20, 0x20, 0x56, 0x31])
            setDatarefHex("laminar/B738/fmc1/Line02_L", [0x20, 0x39, 0x39, 0x2E, 0x35, 0x2F, 0x20, 0x39, 0x39, 0x2E, 0x35, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x30, 0x39])
            setDatarefHex("laminar/B738/fmc1/Line02_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2D, 0x2D, 0x2D])
            setDatarefHex("laminar/B738/fmc1/Line02_X", [0x20, 0x20, 0x20, 0x20, 0x32, 0x36, 0x4B, 0x20, 0x4E, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x56, 0x52])
            setDatarefHex("laminar/B738/fmc1/Line03_L", [0x31, 0x38, 0x2E, 0x34, 0x25, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x32, 0x30])
            setDatarefHex("laminar/B738/fmc1/Line03_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2D, 0x2D, 0x2D])
            setDatarefHex("laminar/B738/fmc1/Line03_X", [0x20, 0x43, 0x47, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x56, 0x32])
            setDatarefHex("laminar/B738/fmc1/Line04_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2D, 0x2D, 0x2D, 0x2E, 0x2D, 0x2F])
            setDatarefHex("laminar/B738/fmc1/Line04_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line04_X", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x47, 0x57, 0x20, 0x20, 0x2F, 0x20, 0x54, 0x4F, 0x57])
            setDatarefHex("laminar/B738/fmc1/Line05_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line05_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line05_X", [0x20, 0x52, 0x55, 0x4E, 0x57, 0x41, 0x59, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line06_L", [0x3C, 0x50, 0x4F, 0x53, 0x20, 0x49, 0x4E, 0x49, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x51, 0x52, 0x48, 0x20, 0x4F, 0x46, 0x46, 0x3E])
            setDatarefHex("laminar/B738/fmc1/Line06_X", [0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x53, 0x45, 0x4C, 0x45, 0x43, 0x54])
            break

        case "FLTPLN_REQ":
            setDatarefHex("laminar/B738/fmc1/Line00_C", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x52, 0x54, 0x45, 0x20, 0x31])
            setDatarefHex("laminar/B738/fmc1/Line00_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line00_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x2F, 0x31])
            setDatarefHex("laminar/B738/fmc1/Line01_I", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line01_L", [0x2D, 0x2D, 0x2D, 0x2D, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x2A, 0x2A, 0x2A, 0x2A])
            setDatarefHex("laminar/B738/fmc1/Line01_X", [0x20, 0x4F, 0x52, 0x49, 0x47, 0x49, 0x4E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x44, 0x45, 0x53, 0x54])
            setDatarefHex("laminar/B738/fmc1/Line02_I", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line02_L", [0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x20, 0x20, 0x20, 0x20, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D])
            setDatarefHex("laminar/B738/fmc1/Line02_X", [0x20, 0x43, 0x4F, 0x20, 0x52, 0x4F, 0x55, 0x54, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x46, 0x4C, 0x54, 0x20, 0x4E, 0x4F, 0x2E])
            setDatarefHex("laminar/B738/fmc1/Line03_I", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x52, 0x45, 0x51, 0x55, 0x45, 0x53, 0x54, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line03_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3E])
            setDatarefHex("laminar/B738/fmc1/Line03_X", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x46, 0x4C, 0x54, 0x20, 0x50, 0x4C, 0x41, 0x4E])
            setDatarefHex("laminar/B738/fmc1/Line04_I", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line04_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line04_X", [0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D])
            setDatarefHex("laminar/B738/fmc1/Line05_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line06_L", [0x3C, 0x4C, 0x4F, 0x41, 0x44, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            break

        case "IDENT":
            fallthrough
        default:
            setDatarefHex("laminar/B738/fmc1/Line00_L", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x49, 0x44, 0x45, 0x4E, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line00_S", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x31, 0x2F, 0x32, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line01_L", [0x37, 0x33, 0x37, 0x2D, 0x38, 0x30, 0x30, 0x57, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x32, 0x36, 0x4B])
            setDatarefHex("laminar/B738/fmc1/Line01_X", [0x20, 0x4D, 0x4F, 0x44, 0x45, 0x4C, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x45, 0x4E, 0x47, 0x20, 0x52, 0x41, 0x54, 0x49, 0x4E, 0x47])
            setDatarefHex("laminar/B738/fmc1/Line02_X", [0x20, 0x4E, 0x41, 0x56, 0x20, 0x44, 0x41, 0x54, 0x41, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x41, 0x43, 0x54, 0x49, 0x56, 0x45])
            setDatarefHex("laminar/B738/fmc1/Line04_L", [0x36, 0x35, 0x30, 0x36, 0x33, 0x30, 0x2D, 0x30, 0x30, 0x32, 0x20, 0x20, 0x28, 0x55, 0x31, 0x34, 0x2E, 0x30, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line04_X", [0x20, 0x4F, 0x50, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20])
            setDatarefHex("laminar/B738/fmc1/Line05_X", [0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x53, 0x55, 0x50, 0x50, 0x20, 0x44, 0x41, 0x54, 0x41])
            setDatarefHex("laminar/B738/fmc1/Line06_L", [0x3C, 0x49, 0x4E, 0x44, 0x45, 0x58, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x50, 0x4F, 0x53, 0x20, 0x49, 0x4E, 0x49, 0x54, 0x3E])
                break
        }
    }
    
    private func setLed(_ ledId: Int, _ state: Bool) {
        guard let pfp = device.pfp else { return }
        pfp.setLed(ledId, state: state)
    }
}

#Preview {
    PFPControlView(device: WinwingDevice(
        id: 0,
        name: "Test PFP",
        type: .pfp,
        productId: 0xBB32,
        isConnected: true
    ))
}

//
//  ContentView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 20/06/2025.
//

import SwiftUI
import Foundation

// Legacy C bridge imports (for backward compatibility)
@_silgen_name("update")
func c_update() -> Void
@_silgen_name("enumerateDevices")
func c_enumerateDevices(_ buffer: UnsafeMutablePointer<CChar>, _ bufferLen: Int32) -> Int32
@_silgen_name("disconnectAll")
func disconnectAll() -> Void
@_silgen_name("setDatarefHexC")
func setDatarefHexC(_ ref: UnsafePointer<CChar>, _ hex: UnsafePointer<UInt8>, _ len: Int32)

// Swift wrapper for setDatarefHex
func setDatarefHex(_ ref: String, _ hex: [UInt8]) {
    ref.withCString { refPtr in
        hex.withUnsafeBufferPointer { hexPtr in
            setDatarefHexC(refPtr, hexPtr.baseAddress!, Int32(hex.count))
        }
    }
}

// Legacy functions for backward compatibility
func enumerateDevices() -> [String] {
    let bufferLen = 1024
    let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: bufferLen)
    defer { buffer.deallocate() }
    let count = c_enumerateDevices(buffer, Int32(bufferLen))
    guard count >= 0 else { return [] }
    let result = String(cString: buffer)
    return result.isEmpty ? [] : result.components(separatedBy: "\n")
}

struct ContentView: View {
    @StateObject private var deviceManager = DeviceManager()
    @State private var refreshTimer: Timer?
    @State private var selectedDevice: WinwingDevice?
    
    var body: some View {
        HStack(spacing: 0) {
            // Sidebar: Device List
            VStack(alignment: .leading, spacing: 0) {
                HStack {
                    Text("Devices")
                        .font(.headline)
                    Spacer()
                    Button(action: { deviceManager.refreshDevices() }) {
                        Image(systemName: "arrow.2.circlepath")
                    }
                }
                .padding([.top, .horizontal])
                
                List(selection: Binding(
                    get: { selectedDevice?.id },
                    set: { selectedId in
                        selectedDevice = deviceManager.devices.first { $0.id == selectedId }
                    }
                )) {
                    ForEach(deviceManager.devices) { device in
                        HStack {
                            VStack(alignment: .leading, spacing: 2) {
                                Text(device.name)
                                    .font(.body)
                                HStack {
                                    Text(device.type.rawValue.capitalized)
                                        .font(.caption)
                                        .foregroundColor(.secondary)
                                    Spacer()
                                }
                            }
                        }
                        .contentShape(Rectangle())
                        .tag(device.id)
                    }
                }
                .frame(width: 220)
                .background(Color(nsColor: .controlBackgroundColor))
            }
            .frame(width: 220)
            .background(Color(nsColor: .controlBackgroundColor))
            .clipShape(RoundedRectangle(cornerRadius: 0))
            .fixedSize(horizontal: true, vertical: false)

            Divider()

            // Main: Device Controls
            Group {
                if let selectedDevice = selectedDevice {
                    deviceControlView(for: selectedDevice)
                } else {
                    VStack {
                        Text("Select a device from the list.")
                            .foregroundColor(.secondary)
                            .padding(.top, 40)
                        Spacer()
                    }
                }
            }
            .padding(32)
            .frame(minWidth: 400, maxWidth: .infinity, alignment: .topLeading)
        }
        .onAppear {
            update();
            refreshTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { _ in
                update();
            }
        }
        .onDisappear {
            refreshTimer?.invalidate()
            refreshTimer = nil
            disconnectAll()
        }
    }
    
    private func update() {
        c_update()
        
        if deviceManager.devices.isEmpty {
            deviceManager.refreshDevices()
        }
        
        if selectedDevice == nil {
            selectedDevice = deviceManager.devices.first
        }
    }
    
    @ViewBuilder
    private func deviceControlView(for device: WinwingDevice) -> some View {
        VStack(alignment: .leading, spacing: 24) {
            switch device.type {
            case .joystick:
                JoystickControlView(device: device)
            case .fmc:
                FMCControlView(device: device)
            case .fcuEfis:
                FCUEfisControlView(device: device)
            case .unknown:
                GenericDeviceView(device: device)
            }
            Spacer()
        }
    }
}

#Preview {
    ContentView()
}

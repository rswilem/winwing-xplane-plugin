//
//  JoystickControlView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 08/07/2025.
//

import SwiftUI

struct JoystickControlView: View {
    let device: WinwingDevice
    @State private var vibration: Double = 0
    @State private var brightness: Double = 0
    
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("Joystick Controls")
                .font(.headline)
                .padding(.top, 8)
            
            // Vibration Row
            HStack(alignment: .center, spacing: 16) {
                Text("Vibration")
                    .frame(width: 80, alignment: .leading)
                Slider(value: $vibration, in: 0...255, step: 1)
                    .frame(width: 160)
                Text("\(Int(vibration))")
                    .frame(width: 36, alignment: .trailing)
                Button(action: { setVibration() }) {
                    Text("Set")
                }
                .buttonStyle(.bordered)
            }
            
            // LED Brightness Row
            HStack(alignment: .center, spacing: 16) {
                Text("LED Brightness")
                    .frame(width: 80, alignment: .leading)
                Slider(value: $brightness, in: 0...255, step: 1)
                    .frame(width: 160)
                Text("\(Int(brightness))")
                    .frame(width: 36, alignment: .trailing)
                Button(action: { setBrightness() }) {
                    Text("Set")
                }
                .buttonStyle(.bordered)
            }
            
            Spacer()
        }
        .padding(.vertical, 8)
        .frame(maxWidth: .infinity, alignment: .topLeading)
    }
    
    private func setVibration() {
        guard let joystick = device.joystick else { return }
        joystick.setVibration(UInt8(vibration))
        // Auto-stop vibration after 1 second
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            joystick.setVibration(0)
        }
    }
    
    private func setBrightness() {
        guard let joystick = device.joystick else { return }
        joystick.setLedBrightness(UInt8(brightness))
    }
}

#Preview {
    JoystickControlView(device: WinwingDevice(
        id: 0,
        name: "Test Joystick",
        type: .joystick,
        productId: 0xBC27,
        isConnected: true
    ))
}

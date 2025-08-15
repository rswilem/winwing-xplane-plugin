//
//  GenericDeviceView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 08/07/2025.
//

import SwiftUI

struct GenericDeviceView: View {
    let device: WinwingDevice
    
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("Unknown Device")
                .font(.headline)
                .padding(.top, 8)
            
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Text("Name:")
                    Text(device.name)
                        .fontWeight(.medium)
                }
                
                HStack {
                    Text("Product ID:")
                    Text(String(format: "0x%04X", device.productId))
                        .font(.monospaced(.body)())
                }
            }
            .padding()
            .background(Color(nsColor: .controlBackgroundColor))
            .cornerRadius(8)
            
            Text("This device type is not yet supported with specific controls.")
                .foregroundColor(.secondary)
                .padding(.top, 8)
            
            Text("However, you can access the device directly:")
                .foregroundColor(.secondary)
                .font(.caption)
                .padding(.top, 4)
            
            Text("device.disconnect(), device.update()")
                .font(.caption)
                .fontDesign(.monospaced)
                .foregroundColor(.blue)
                .padding(.top, 2)
            
            if device.type == .joystick {
                Text("device.joystick?.setVibration(255)")
                    .font(.caption)
                    .fontDesign(.monospaced)
                    .foregroundColor(.blue)
                    .padding(.top, 2)
            } else if device.type == .fmc {
                Text("device.fmc?.clearDisplay(), device.fmc?.showBackground(8)")
                    .font(.caption)
                    .fontDesign(.monospaced)
                    .foregroundColor(.blue)
                    .padding(.top, 2)
            }
            
            Spacer()
        }
        .padding(.vertical, 8)
        .frame(maxWidth: .infinity, alignment: .topLeading)
    }
}

#Preview {
    GenericDeviceView(device: WinwingDevice(
        id: 0,
        name: "Unknown Device",
        type: .unknown,
        productId: 0x1234,
        isConnected: true
    ))
}

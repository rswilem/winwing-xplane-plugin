//
//  DeviceModel.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 08/07/2025.
//

import SwiftUI
import Foundation
import Combine

// C bridge imports for device-specific functions
@_silgen_name("getDeviceCount")
func c_getDeviceCount() -> Int32
@_silgen_name("getDeviceName")
func c_getDeviceName(_ deviceIndex: Int32) -> UnsafePointer<CChar>?
@_silgen_name("getDeviceType")
func c_getDeviceType(_ deviceIndex: Int32) -> UnsafePointer<CChar>?
@_silgen_name("getDeviceProductId")
func c_getDeviceProductId(_ deviceIndex: Int32) -> UInt16
@_silgen_name("isDeviceConnected")
func c_isDeviceConnected(_ deviceIndex: Int32) -> Bool

// Device handle access
@_silgen_name("getDeviceHandle")
func c_getDeviceHandle(_ deviceIndex: Int32) -> UnsafeRawPointer?
@_silgen_name("getJoystickHandle")
func c_getJoystickHandle(_ deviceIndex: Int32) -> UnsafeRawPointer?
@_silgen_name("getMCDUHandle")
func c_getMCDUHandle(_ deviceIndex: Int32) -> UnsafeRawPointer?
@_silgen_name("getPFPHandle")
func c_getPFPHandle(_ deviceIndex: Int32) -> UnsafeRawPointer?

// Generic device functions via handle
@_silgen_name("device_connect")
func c_device_connect(_ handle: UnsafeRawPointer) -> Bool
@_silgen_name("device_disconnect")
func c_device_disconnect(_ handle: UnsafeRawPointer) -> Void
@_silgen_name("device_update")
func c_device_update(_ handle: UnsafeRawPointer) -> Void

// Joystick functions via handle
@_silgen_name("joystick_setVibration")
func c_joystick_setVibration(_ handle: UnsafeRawPointer, _ vibration: UInt8) -> Bool
@_silgen_name("joystick_setLedBrightness")
func c_joystick_setLedBrightness(_ handle: UnsafeRawPointer, _ brightness: UInt8) -> Bool

// MCDU functions via handle
@_silgen_name("mcdu_clear2")
func c_mcdu_clear2(_ handle: UnsafeRawPointer, _ displayId: Int32) -> Void
@_silgen_name("mcdu_clear")
func c_mcdu_clear(_ handle: UnsafeRawPointer) -> Void
@_silgen_name("mcdu_setLed")
func c_mcdu_setLed(_ handle: UnsafeRawPointer, _ ledId: Int32, _ value: UInt8) -> Bool
@_silgen_name("mcdu_setLedBrightness")
func c_mcdu_setLedBrightness(_ handle: UnsafeRawPointer, _ ledId: Int32, _ brightness: UInt8) -> Void

// PFP functions via handle
@_silgen_name("pfp_clear2")
func c_pfp_clear2(_ handle: UnsafeRawPointer, _ displayId: Int32) -> Void
@_silgen_name("pfp_clear")
func c_pfp_clear(_ handle: UnsafeRawPointer) -> Void
@_silgen_name("pfp_setLed")
func c_pfp_setLed(_ handle: UnsafeRawPointer, _ ledId: Int32, _ value: UInt8) -> Bool
@_silgen_name("pfp_setLedBrightness")
func c_pfp_setLedBrightness(_ handle: UnsafeRawPointer, _ ledId: Int32, _ brightness: UInt8) -> Void

enum DeviceType: String, CaseIterable {
    case joystick = "joystick"
    case mcdu = "mcdu"
    case pfp = "pfp"
    case unknown = "unknown"
}

struct WinwingDevice: Identifiable, Equatable, Hashable {
    let id: Int
    let name: String
    let type: DeviceType
    let productId: UInt16
    var isConnected: Bool
    
    // Device handles (cached for performance)
    private let deviceHandle: UnsafeRawPointer?
    private let joystickHandle: UnsafeRawPointer?
    private let mcduHandle: UnsafeRawPointer?
    private let pfpHandle: UnsafeRawPointer?
    
    init(id: Int, name: String, type: DeviceType, productId: UInt16, isConnected: Bool) {
        self.id = id
        self.name = name
        self.type = type
        self.productId = productId
        self.isConnected = isConnected
        
        // Cache device handles
        self.deviceHandle = c_getDeviceHandle(Int32(id))
        self.joystickHandle = c_getJoystickHandle(Int32(id))
        self.mcduHandle = c_getMCDUHandle(Int32(id))
        self.pfpHandle = c_getPFPHandle(Int32(id))
    }
    
    static func == (lhs: WinwingDevice, rhs: WinwingDevice) -> Bool {
        return lhs.id == rhs.id && 
               lhs.name == rhs.name && 
               lhs.type == rhs.type && 
               lhs.productId == rhs.productId && 
               lhs.isConnected == rhs.isConnected
    }
    
    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
        hasher.combine(name)
        hasher.combine(type)
        hasher.combine(productId)
        hasher.combine(isConnected)
    }
    
    // Device wrapper methods
    @discardableResult
    func connect() -> Bool {
        guard let handle = deviceHandle else { return false }
        return c_device_connect(handle)
    }
    
    func disconnect() {
        guard let handle = deviceHandle else { return }
        c_device_disconnect(handle)
    }
    
    func update() {
        guard let handle = deviceHandle else { return }
        c_device_update(handle)
    }
    
    // Joystick wrapper methods
    var joystick: JoystickWrapper? {
        guard let handle = joystickHandle else { return nil }
        return JoystickWrapper(handle: handle)
    }
    
    // MCDU wrapper methods  
    var mcdu: MCDUWrapper? {
        guard let handle = mcduHandle else { return nil }
        return MCDUWrapper(handle: handle)
    }
    
    // PFP wrapper methods
    var pfp: PFPWrapper? {
        guard let handle = pfpHandle else { return nil }
        return PFPWrapper(handle: handle)
    }
}

// Swift wrapper for Joystick functions
struct JoystickWrapper {
    private let handle: UnsafeRawPointer
    
    init(handle: UnsafeRawPointer) {
        self.handle = handle
    }
    
    @discardableResult
    func setVibration(_ vibration: UInt8) -> Bool {
        return c_joystick_setVibration(handle, vibration)
    }
    
    @discardableResult
    func setLedBrightness(_ brightness: UInt8) -> Bool {
        return c_joystick_setLedBrightness(handle, brightness)
    }
}

// Swift wrapper for MCDU functions
struct MCDUWrapper {
    private let handle: UnsafeRawPointer
    
    // LED IDs based on the C++ enum
    enum LEDId: Int {
        case backlight = 0
        case screenBacklight = 1
        case fail = 8
        case fm = 9
        case mcdu = 10
        case menu = 11
        case fm1 = 12
        case ind = 13
        case rdy = 14
        case status = 15
        case fm2 = 16
    }
    
    init(handle: UnsafeRawPointer) {
        self.handle = handle
    }
    
    func clear() {
        c_mcdu_clear(handle)
    }
    
    func clear2(_ displayId: Int) {
        c_mcdu_clear2(handle, Int32(displayId))
    }
    
    @discardableResult
    func setLed(_ ledId: LEDId, value: UInt8) -> Bool {
        return c_mcdu_setLed(handle, Int32(ledId.rawValue), value)
    }
    
    @discardableResult
    func setLed(_ ledId: LEDId, state: Bool) -> Bool {
        return setLed(ledId, value: state ? 255 : 0)
    }
    
    @discardableResult
    func setLed(_ ledId: Int, value: UInt8) -> Bool {
        return c_mcdu_setLed(handle, Int32(ledId), value)
    }
    
    @discardableResult
    func setLed(_ ledId: Int, state: Bool) -> Bool {
        return setLed(ledId, value: state ? 255 : 0)
    }
    
    func setLedBrightness(_ ledId: LEDId, brightness: UInt8) {
        c_mcdu_setLedBrightness(handle, Int32(ledId.rawValue), brightness)
    }
    
    func setLedBrightness(_ ledId: Int, brightness: UInt8) {
        c_mcdu_setLedBrightness(handle, Int32(ledId), brightness)
    }
    
    // Convenience methods for common LEDs
    func setBacklight(_ brightness: UInt8) {
        setLedBrightness(.backlight, brightness: brightness)
    }
    
    func setScreenBacklight(_ brightness: UInt8) {
        setLedBrightness(.screenBacklight, brightness: brightness)
    }
}

// Swift wrapper for PFP functions
struct PFPWrapper {
    private let handle: UnsafeRawPointer
    
    // LED IDs based on the C++ enum (same as MCDU)
    enum LEDId: Int {
        case backlight = 0
        case screenBacklight = 1
        case fail = 8
        case fm = 9
        case mcdu = 10
        case menu = 11
        case fm1 = 12
        case ind = 13
        case rdy = 14
        case status = 15
        case fm2 = 16
    }
    
    init(handle: UnsafeRawPointer) {
        self.handle = handle
    }
    
    func clear() {
        c_pfp_clear(handle)
    }
    
    func clear2(_ displayId: Int) {
        c_pfp_clear2(handle, Int32(displayId))
    }
    
    @discardableResult
    func setLed(_ ledId: LEDId, value: UInt8) -> Bool {
        return c_pfp_setLed(handle, Int32(ledId.rawValue), value)
    }
    
    @discardableResult
    func setLed(_ ledId: LEDId, state: Bool) -> Bool {
        return setLed(ledId, value: state ? 255 : 0)
    }
    
    @discardableResult
    func setLed(_ ledId: Int, value: UInt8) -> Bool {
        return c_pfp_setLed(handle, Int32(ledId), value)
    }
    
    @discardableResult
    func setLed(_ ledId: Int, state: Bool) -> Bool {
        return setLed(ledId, value: state ? 255 : 0)
    }
    
    func setLedBrightness(_ ledId: LEDId, brightness: UInt8) {
        c_pfp_setLedBrightness(handle, Int32(ledId.rawValue), brightness)
    }
    
    func setLedBrightness(_ ledId: Int, brightness: UInt8) {
        c_pfp_setLedBrightness(handle, Int32(ledId), brightness)
    }
    
    // Convenience methods for common LEDs
    func setBacklight(_ brightness: UInt8) {
        setLedBrightness(.backlight, brightness: brightness)
    }
    
    func setScreenBacklight(_ brightness: UInt8) {
        setLedBrightness(.screenBacklight, brightness: brightness)
    }
}

@MainActor
class DeviceManager: ObservableObject {
    @Published var devices: [WinwingDevice] = []
    @Published var selectedDeviceId: Int?
    
    var selectedDevice: WinwingDevice? {
        guard let id = selectedDeviceId else { return nil }
        return devices.first { $0.id == id }
    }
    
    func refreshDevices() {
        let count = Int(c_getDeviceCount())
        var newDevices: [WinwingDevice] = []
        
        for i in 0..<count {
            guard let namePtr = c_getDeviceName(Int32(i)),
                  let typePtr = c_getDeviceType(Int32(i)) else {
                continue
            }
            
            let name = String(cString: namePtr)
            let typeString = String(cString: typePtr)
            let type = DeviceType(rawValue: typeString) ?? .unknown
            let productId = c_getDeviceProductId(Int32(i))
            let isConnected = c_isDeviceConnected(Int32(i))
            
            let device = WinwingDevice(
                id: i,
                name: name,
                type: type,
                productId: productId,
                isConnected: isConnected
            )
            newDevices.append(device)
        }
        
        // Only update if devices actually changed (for performance)
        let devicesChanged = devices.count != newDevices.count || 
                           !devices.elementsEqual(newDevices, by: ==)
        
        if devicesChanged {
            devices = newDevices
            
            // Update selected device or select first if none selected
            if let selectedId = selectedDeviceId {
                // Check if selected device still exists
                if !devices.contains(where: { $0.id == selectedId }) {
                    selectedDeviceId = devices.first?.id
                }
            } else if !devices.isEmpty {
                selectedDeviceId = devices.first?.id
            }
        }
    }
    
    func selectDevice(_ device: WinwingDevice) {
        selectedDeviceId = device.id
    }
}

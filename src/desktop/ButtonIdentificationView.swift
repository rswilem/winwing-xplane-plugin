//
//  ButtonIdentificationView.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 08/11/2025.
//

import SwiftUI
import UniformTypeIdentifiers
import Combine

// C bridge imports for button listening
@_silgen_name("setButtonListeningMode")
func c_setButtonListeningMode(_ enabled: Bool) -> Void
@_silgen_name("getButtonListeningMode")
func c_getButtonListeningMode() -> Bool
@_silgen_name("hasButtonPressed")
func c_hasButtonPressed() -> Bool
@_silgen_name("getLastPressedButtonId")
func c_getLastPressedButtonId() -> Int32
@_silgen_name("getLastPressedProductId")
func c_getLastPressedProductId() -> Int32
@_silgen_name("clearLastPressedButton")
func c_clearLastPressedButton() -> Void

class ButtonIdentificationViewModel: ObservableObject {
    @Published var buttonMappings: [ButtonMapping] = []
    @Published var isListening: Bool = false
    @Published var isWaitingForButton: Bool = false
    @Published var currentButtonDescription: String = ""
    @Published var highlightedButtonId: Int? = nil
    
    private var checkTimer: Timer?
    
    func startListening() {
        isWaitingForButton = true
        c_setButtonListeningMode(true)
        
        // Poll for button presses
        checkTimer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { [weak self] _ in
            self?.checkForButtonPress()
        }
    }
    
    func stopListening() {
        isWaitingForButton = false
        checkTimer?.invalidate()
        checkTimer = nil
        c_setButtonListeningMode(false)
    }
    
    private func checkForButtonPress() {
        if c_hasButtonPressed() {
            let buttonId = Int(c_getLastPressedButtonId())
            let productId = Int(c_getLastPressedProductId())
            c_clearLastPressedButton()
            
            DispatchQueue.main.async {
                if self.buttonMappings.contains(where: { $0.buttonId == buttonId && $0.productId == productId }) {
                    self.highlightedButtonId = buttonId
                    DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                        self.highlightedButtonId = nil
                    }
                } else {
                    let newMapping = ButtonMapping(buttonId: buttonId, productId: productId, description: "")
                    self.buttonMappings.append(newMapping)
                }
            }
        }
    }
    
    func updateDescription(for buttonId: Int, productId: Int, description: String) {
        if let index = buttonMappings.firstIndex(where: { $0.buttonId == buttonId && $0.productId == productId }) {
            buttonMappings[index].description = description
        }
    }
    
    func removeMapping(at offsets: IndexSet) {
        buttonMappings.remove(atOffsets: offsets)
    }
    
    func exportToJSON() -> String? {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        
        guard let data = try? encoder.encode(buttonMappings) else {
            return nil
        }
        
        return String(data: data, encoding: .utf8)
    }
    
    func clearAll() {
        buttonMappings.removeAll()
    }
}

struct ButtonIdentificationView: View {
    @StateObject private var viewModel = ButtonIdentificationViewModel()
    @Environment(\.dismiss) var dismiss
    @State private var showingSavePanel = false
    @State private var showingClearAlert = false
    
    var body: some View {
        VStack(spacing: 0) {
            // Header
            HStack {
                Text("Button Identification")
                    .font(.title2)
                    .fontWeight(.semibold)
                
                Spacer()
                
                Button("Done") {
                    viewModel.stopListening()
                    dismiss()
                }
            }
            .padding()
            
            Divider()
            
            // Instructions
            VStack(alignment: .leading, spacing: 8) {
                Text("Instructions:")
                    .font(.headline)
                
                Text("1. Click \"Start Listening\" below")
                Text("2. Press a button on your unknown device")
                Text("3. The button ID will appear in the table")
                Text("4. Enter a description for each button")
                Text("5. Export the mappings as JSON when done")
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding()
            .background(Color(nsColor: .controlBackgroundColor))
            
            // Listening control
            HStack {
                if viewModel.isWaitingForButton {
                    HStack(spacing: 8) {
                        ProgressView()
                            .scaleEffect(0.7)
                        Text("Waiting for button press...")
                            .foregroundColor(.secondary)
                    }
                    
                    Spacer()
                    
                    Button("Stop Listening") {
                        viewModel.stopListening()
                    }
                    .buttonStyle(.borderedProminent)
                } else {
                    Button("Start Listening") {
                        viewModel.startListening()
                    }
                    .buttonStyle(.borderedProminent)
                    
                    Spacer()
                }
            }
            .padding()
            
            Divider()
            
            // Button mappings table
            if viewModel.buttonMappings.isEmpty {
                VStack {
                    Spacer()
                    Text("No buttons identified yet")
                        .foregroundColor(.secondary)
                        .font(.title3)
                    Text("Start listening and press buttons on your device")
                        .foregroundColor(.secondary)
                        .font(.caption)
                    Spacer()
                }
            } else {
                VStack(spacing: 0) {
                    // Table header
                    HStack {
                        Text("Product ID")
                            .font(.headline)
                            .frame(width: 100, alignment: .leading)
                        
                        Text("Button ID")
                            .font(.headline)
                            .frame(width: 100, alignment: .leading)
                        
                        Text("Description")
                            .font(.headline)
                            .frame(maxWidth: .infinity, alignment: .leading)
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 8)
                    .background(Color(nsColor: .controlBackgroundColor))
                    
                    Divider()
                    
                    // Table content
                    ScrollView {
                        LazyVStack(spacing: 0) {
                            ForEach(viewModel.buttonMappings) { mapping in
                                HStack {
                                    Text(String(format: "0x%04X", mapping.productId))
                                        .frame(width: 100, alignment: .leading)
                                        .foregroundColor(.secondary)
                                        .font(.system(.body, design: .monospaced))
                                    
                                    Text("\(mapping.buttonId)")
                                        .frame(width: 100, alignment: .leading)
                                        .foregroundColor(.primary)
                                    
                                    TextField("Enter description...", text: Binding(
                                        get: { mapping.description },
                                        set: { newValue in
                                            viewModel.updateDescription(for: mapping.buttonId, productId: mapping.productId, description: newValue)
                                        }
                                    ))
                                    .textFieldStyle(.roundedBorder)
                                }
                                .padding(.horizontal)
                                .padding(.vertical, 8)
                                .background(viewModel.highlightedButtonId == mapping.buttonId ? Color.yellow.opacity(0.3) : Color.clear)
                                .animation(.easeInOut(duration: 0.3), value: viewModel.highlightedButtonId)
                                
                                Divider()
                            }
                            .onDelete { indexSet in
                                viewModel.removeMapping(at: indexSet)
                            }
                        }
                    }
                }
            }
            
            Divider()
            
            // Bottom actions
            HStack {
                Button("Clear All") {
                    showingClearAlert = true
                }
                .disabled(viewModel.buttonMappings.isEmpty)
                
                Spacer()
                
                Button("Export as JSON") {
                    showingSavePanel = true
                }
                .disabled(viewModel.buttonMappings.isEmpty)
                .buttonStyle(.borderedProminent)
            }
            .padding()
        }
        .frame(minWidth: 600, minHeight: 500)
        .alert("Clear All Mappings", isPresented: $showingClearAlert) {
            Button("Cancel", role: .cancel) {}
            Button("Clear All", role: .destructive) {
                viewModel.clearAll()
            }
        } message: {
            Text("Are you sure you want to clear all button mappings? This action cannot be undone.")
        }
        .fileExporter(
            isPresented: $showingSavePanel,
            document: ButtonMappingDocument(mappings: viewModel.buttonMappings),
            contentType: .json,
            defaultFilename: "button_mappings.json"
        ) { result in
            switch result {
            case .success(let url):
                print("Saved to \(url)")
            case .failure(let error):
                print("Error saving: \(error.localizedDescription)")
            }
        }
    }
}

// FileDocument for exporting JSON
struct ButtonMappingDocument: FileDocument {
    static var readableContentTypes: [UTType] { [.json] }
    
    var mappings: [ButtonMapping]
    
    init(mappings: [ButtonMapping]) {
        self.mappings = mappings
    }
    
    init(configuration: ReadConfiguration) throws {
        guard let data = configuration.file.regularFileContents else {
            throw CocoaError(.fileReadCorruptFile)
        }
        mappings = try JSONDecoder().decode([ButtonMapping].self, from: data)
    }
    
    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let data = try encoder.encode(mappings)
        return FileWrapper(regularFileWithContents: data)
    }
}

#Preview {
    ButtonIdentificationView()
}

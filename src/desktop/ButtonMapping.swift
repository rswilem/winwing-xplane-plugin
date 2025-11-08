//
//  ButtonMapping.swift
//  WinwingDesktop
//
//  Created by Ramon Swilem on 08/11/2025.
//

import Foundation

struct ButtonMapping: Identifiable, Codable {
    var id: String
    var buttonId: Int
    var productId: Int
    var description: String
    
    init(buttonId: Int, productId: Int, description: String = "") {
        self.id = "\(productId)_\(buttonId)"
        self.buttonId = buttonId
        self.productId = productId
        self.description = description
    }
}

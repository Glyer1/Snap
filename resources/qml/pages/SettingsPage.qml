import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Snap 0.1

Window {
    id: settingsWindow
    width: 480
    height: 400
    visible: false
    title: "设置"
    color: "#FFFFFF"
    modality: Qt.ApplicationModal

    property string currentCompany: "DeepSeek"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: "API 配置"
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
        }

        // 公司名（只读显示）
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label { text: "服务商:"; font.pixelSize: 13; Layout.preferredWidth: 80 }
            TextField {
                id: companyInput
                text: "DeepSeek"
                readOnly: true
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#F5F7FA"
                    radius: 4
                    border.color: "#E2E8F0"
                    border.width: 1
                }
            }
        }

        // API Key
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label { text: "API Key:"; font.pixelSize: 13; Layout.preferredWidth: 80 }
            TextField {
                id: apiKeyInput
                Layout.fillWidth: true
                placeholderText: "输入你的 API Key"
                echoMode: TextField.PasswordEchoOnEdit
                background: Rectangle {
                    color: "#FFFFFF"
                    radius: 4
                    border.color: "#D1D5DB"
                    border.width: 1
                }
            }
        }

        // Base URL
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label { text: "Base URL:"; font.pixelSize: 13; Layout.preferredWidth: 80 }
            TextField {
                id: urlInput
                text: "https://api.deepseek.com/anthropic/v1/messages"
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#FFFFFF"
                    radius: 4
                    border.color: "#D1D5DB"
                    border.width: 1
                }
            }
        }

        // 模型选择
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Label { text: "模型:"; font.pixelSize: 13; Layout.preferredWidth: 80 }
            ComboBox {
                id: modelCombo
                model: ["deepseek-v4-flash", "deepseek-v4-pro"]
                currentIndex: 0
                Layout.fillWidth: true
            }
        }

        // 保存 / 取消 按钮
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Layout.topMargin: 8

            Button {
                text: "保存"
                Layout.fillWidth: true
                onClicked: {
                    var key = apiKeyInput.text.trim()
                    if (!key) {
                        // 简单提示，实际可以用 Toast
                        console.warn("API Key 不能为空")
                        return
                    }

                    AppCore.saveApiSettings(
                        companyInput.text,
                        key,
                        urlInput.text.trim(),
                        modelCombo.currentText
                    )
                    settingsWindow.close()
                }
            }

            Button {
                text: "取消"
                Layout.fillWidth: true
                onClicked: settingsWindow.close()
            }
        }

        // 分隔线
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#E2E8F0"
        }

        // 加载配置状态提示
        Label {
            text: "提示：API Key 会保存在系统凭据中，URL 和模型保存在本地配置"
            font.pixelSize: 11
            color: "#999999"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }

    function loadSettings() {
        // 从 AppCore 加载已有的配置
        var company = "DeepSeek"
        var url = AppCore.getBaseUrlForCompany(company)
        var model = AppCore.getModelForCompany(company)
        var key = AppCore.getApiKeyForCompany(company)

        companyInput.text = company
        urlInput.text = url
        modelCombo.currentIndex = modelCombo.find(model)

        if (key) {
            apiKeyInput.text = key
        }
    }
}

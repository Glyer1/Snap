import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Snap 0.1
import "./pages"

Window {
    width: 400
    height: 600
    visible: true
    title: qsTr("SnapApiHelper")
    color: "#FFFFFF"

    property var settingsPage: null
    property string currentCompany: "DeepSeek"
    property string currentBaseUrl: "https://api.deepseek.com/anthropic/v1/messages"
    property string currentModel: "deepseek-v4-flash"

    // 模拟数据
    ListModel {
        id: apiModel
        ListElement {
            name: "QFile::open"
            desc: "打开文件"
            example: "file.open(QIODevice::ReadOnly)"
            params: "QIODevice::OpenMode mode"
            detail: "以指定模式打开文件，返回bool表示是否成功"
        }
        ListElement {
            name: "QString::toInt"
            desc: "字符串转整数"
            example: "QString(\"123\").toInt()"
            params: "bool *ok = nullptr, int base = 10"
            detail: "将字符串转换为int，可设置进制"
        }
        ListElement {
            name: "QByteArray::toHex"
            desc: "转十六进制"
            example: "ba.toHex()"
            params: "无参数"
            detail: "将字节数组转换为十六进制字符串"
        }
        ListElement {
            name: "QJsonDocument::fromJson"
            desc: "解析JSON"
            example: "QJsonDocument::fromJson(jsonData)"
            params: "const QByteArray &json, QJsonParseError *error = nullptr"
            detail: "从QByteArray解析JSON文档"
        }
        ListElement {
            name: "QThread::msleep"
            desc: "线程休眠毫秒"
            example: "QThread::msleep(100)"
            params: "unsigned long msecs"
            detail: "当前线程休眠指定毫秒"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        //搜索区域
        Label {
            text: "请输入需要查询的问题或API："
            font.pixelSize: 13
            color: "#333333"
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: searchInput
                Layout.fillWidth: true
                placeholderText: "例如：QFile 读取文件"
                placeholderTextColor: "#999999"
                color: "#1A2332"
                font.pixelSize: 13
                selectByMouse: true
                onAccepted: performSearch()

                background: Rectangle {
                    color: "#FFFFFF"
                    radius: 4
                    border.color: "#D1D5DB"
                    border.width: 1
                }
            }

            Button {
                contentItem: Text{
                    text: "查询"
                    color: "black"
                }

                onClicked: performSearch()
            }
        }

        //列表
        Label {
            text: "相关 API："
            font.pixelSize: 12
            color: "#666666"
            Layout.fillWidth: true
            visible: apiListView.count > 0
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "#F5F7FA"
            radius: 6
            border.color: "#E2E8F0"
            border.width: 1
            visible: apiListView.count > 0

            ListView {
                id: apiListView
                anchors.fill: parent
                anchors.margins: 4
                model: apiModel
                clip: true
                delegate: ItemDelegate {
                    width: parent.width
                    height: 32
                    text: name + " — " + desc
                    font.pixelSize: 12
                    onClicked: {
                        detailName.text = name
                        detailDesc.text = desc
                        detailParams.text = params
                        detailExample.text = example
                        detailDetail.text = detail
                    }
                }
            }
        }

        //详情区域
        Label {
            text: "详细信息"
            font.pixelSize: 12
            color: "#666666"
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FAFBFC"
            radius: 6
            border.color: "#E2E8F0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4

                // API名称 + 复制按钮
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        id: detailName
                        text: "选择上方的 API 查看详情"
                        font.pixelSize: 15
                        font.bold: true
                        color: "#1A2332"
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }

                    Button {
                        contentItem: Text{
                            text: "复制"
                            color: "black"
                        }
                        font.pixelSize: 11
                        enabled: detailName.text !== "选择上方的 API 查看详情"
                        onClicked: {
                            var full = detailName.text + "\n" +
                                       "参数: " + detailParams.text + "\n" +
                                       "示例: " + detailExample.text + "\n" +
                                       "说明: " + detailDetail.text
                            // 实际复制用 clipboard
                            console.log("复制内容:", full)
                        }
                    }
                }

                Text {
                    id: detailDesc
                    text: "解释: "
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }

                Text {
                    id: neededHead
                    text: "需要的头文件: "
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }

                Text {
                    id: detailParams
                    text: "参数: "
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }

                Text {
                    id: detailExample
                    text: "示例: "
                    font.pixelSize: 12
                    color: "#1A2332"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    font.family: "Courier New"
                }

                Text {
                    id: detailDetail
                    text: "说明: "
                    font.pixelSize: 12
                    color: "#475569"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    //设置按钮
    Button {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        text: "⚙️ 设置"
        flat: true
        onClicked: {
            if (settingsPage) {
                settingsPage.show()
                settingsPage.loadSettings()
            } else {
                settingsLoader.active = true
            }
        }
    }

    Loader {
        id: settingsLoader
        active: false
        source: "./pages/SettingsPage.qml"
        onLoaded: {
            settingsPage = item
            settingsPage.show()
            if (typeof settingsPage.loadSettings === "function") {
                settingsPage.loadSettings()
            }
        }
    }

    //创建完成组件直接绑定信号自动让函数将apimodel更新
    Component.onCompleted: {

        loadConfig()

        AppCore.settingsChanged.connect(loadConfig)

        AppCore.searchResultReady.connect(function(results)
        {
            apiModel.clear()
            if (results.length > 0) {
                var item = results[0]
                if (item.success) {
                    // 把 AI 返回的完整内容显示在详情面板
                    apiModel.append({
                        name: "AI 搜索结果",
                        desc: "点击查看完整回答",
                        example: "",
                        params: "",
                        detail: item.content
                    })
                    // 自动选中以展示详情
                    detailName.text = item.name || "API 名称"
                    neededHead.text = "需要的头文件: " + (item.head || "无")
                    detailDesc.text = "解释: " + (item.desc || "无")
                    detailParams.text = "参数: " + (item.params || "无")
                    detailExample.text = "示例: " + (item.example || "无")
                    detailDetail.text = "说明: " + (item.detail || "无")
                } else {
                    apiModel.append({
                        name: "查询失败",
                        desc: item.errorMessage || "未知错误",
                        example: "",
                        params: "",
                        detail: item.errorMessage || "未知错误"
                    })
                }
            }
        })
    }

    //查询
    function performSearch() {
        var keyword = searchInput.text.trim().toLowerCase()
        if (!keyword) {
            apiModel.clear()
            return
        }

        //搜索
        apiModel.clear()
        AppCore.searchApi(searchInput.text, currentCompany, currentBaseUrl, currentModel)

        console.log("currentModel:", currentModel)
        console.log("currentUrl:", currentBaseUrl)
    }

    //加载配置
    function loadConfig() {
        var company = "DeepSeek"
        currentCompany = company
        currentBaseUrl = AppCore.getBaseUrlForCompany(company)
        currentModel = AppCore.getModelForCompany(company)
        // 如果读取到的 URL 是旧格式（没有 /v1/messages），自动补全
        if (currentBaseUrl === "https://api.deepseek.com/anthropic") {
            currentBaseUrl = "https://api.deepseek.com/anthropic/v1/messages"
        }
    }
}



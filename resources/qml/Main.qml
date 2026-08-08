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
    property var historyPage: null
    property string currentCompany: "DeepSeek"
    property string currentBaseUrl: "https://api.deepseek.com/anthropic/v1/messages"
    property string currentModel: "deepseek-v4-flash"

    // 模拟数据
    ListModel {
        id: apiModel
        ListElement {
            name: "QFile::open"
            head: ""
            desc: "打开文件"
            example: "file.open(QIODevice::ReadOnly)"
            params: "QIODevice::OpenMode mode"
            detail: "以指定模式打开文件，返回bool表示是否成功"
        }
        ListElement {
            name: "QString::toInt"
            head: ""
            desc: "字符串转整数"
            example: "QString(\"123\").toInt()"
            params: "bool *ok = nullptr, int base = 10"
            detail: "将字符串转换为int，可设置进制"
        }
        ListElement {
            name: "QByteArray::toHex"
            head: ""
            desc: "转十六进制"
            example: "ba.toHex()"
            params: "无参数"
            detail: "将字节数组转换为十六进制字符串"
        }
        ListElement {
            name: "QJsonDocument::fromJson"
            head: ""
            desc: "解析JSON"
            example: "QJsonDocument::fromJson(jsonData)"
            params: "const QByteArray &json, QJsonParseError *error = nullptr"
            detail: "从QByteArray解析JSON文档"
        }
        ListElement {
            name: "QThread::msleep"
            head: ""
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
                        neededHead.text = "<b>需要的头文件:</b> " + (head || "无")
                        detailDesc.text = "<b>解释:</b> " + (desc || "无")
                        detailParams.text = "<b>参数:</b> " + (params || "无")
                        detailExample.text = "<b>示例:</b> " + (example || "无")
                        detailDetail.text = "<b>说明:</b> " + (detail || "无")
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
                        text: "<b>请选择上方的 API 查看详情</b>"
                        font.pixelSize: 15
                        font.bold: true
                        color: "#1A2332"
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        textFormat: Text.RichText
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
                                       "需要的头文件：" + stripHtml(neededHead.text) + "\n" +
                                       "解释：" + stripHtml(detailDesc.text) + "\n" +
                                       "参数: " + stripHtml(detailParams.text) + "\n" +
                                       "示例: " + stripHtml(detailExample.text) + "\n" +
                                       "说明: " + stripHtml(detailDetail.text)

                            clipboradUtilsManager.setText(full);
                            console.log("复制内容:", full)
                        }
                    }
                }

                Text {
                    id: detailDesc
                    text: "<b>解释</b>: 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: neededHead
                    text: "<b>需要的头文件</b>: 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailParams
                    text: "<b>参数</b>: 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailExample
                    text: "<b>示例</b>: 暂无数据"
                    font.pixelSize: 12
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailDetail
                    text: "<b>说明:</b> 暂无数据"
                    font.pixelSize: 12
                    color: "#475569"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
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

        Button {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 16
            text: "历史记录"
            flat: true
            onClicked: {
                if(historyPage)
                {
                    historyPage.show()
                    historyPage.load()
                }
                else
                {
                    historyLoader.active = true;
                }
            }
        }

        Loader {
            id: historyLoader
            active: false
            source: "./pages/HistoryPage.qml"
            onLoaded: {
                historyPage = item
                historyPage.show()
                if(typeof historyPage.load() === "function"){
                    historyPage.load()
                }
            }
        }


    //创建完成组件直接绑定信号自动让函数将apimodel更新
    Component.onCompleted: {
        loadConfig()
        AppCore.loadApiKey("DeepSeek")
        AppCore.settingsChanged.connect(loadConfig)

        AppCore.searchResultReady.connect(function(results) {
            apiModel.clear()
            if (results.length > 0) {
                // 填充列表
                for (var i = 0; i < results.length; i++) {
                    var item = results[i]
                    apiModel.append({
                        name: item.name || "未命名",
                        desc: item.desc || "",
                        head: item.head || "",
                        params: item.params || "",
                        example: item.example || "",
                        detail: item.detail || ""
                    })
                }
                // 自动显示第一条
                var first = results[0]
                detailName.text = first.name || "未命名"
                neededHead.text = "<b>需要的头文件:</b> " + (first.head || "无")
                detailDesc.text = "<b>解释:</b> " + (first.desc || "无")
                detailParams.text = "<b>参数:</b> " + (first.params || "无")
                detailExample.text = "<b>示例:</b> " + (first.example || "无")
                detailDetail.text = "<b>说明:</b> " + (first.detail || "无")
            } else {
                // 无结果
                apiModel.append({
                    name: "无结果",
                    desc: "未找到相关信息",
                    head: "",
                    params: "",
                    example: "",
                    detail: "请尝试其他关键词"
                })
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

    //去除html字符串
    function stripHtml(text) {
        return text.replace(/<[^>]*>/g, "")
    }
}



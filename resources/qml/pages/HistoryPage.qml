// pages/HistoryPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Snap 0.1

Window {
    id: historyWindow
    width: 400
    height: 600
    visible: true
    title: qsTr("搜索历史")
    color: "#FFFFFF"
    modality: Qt.ApplicationModal

    property string currentKeyword: ""

    // 历史数据模型
    ListModel {
        id: historyModel
    }

    // 搜索过滤后的模型（只用于显示）
    ListModel {
        id: filteredModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // ===== 顶部：搜索栏 + 返回按钮 =====
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: searchInput
                Layout.fillWidth: true
                placeholderText: "搜索历史 API..."
                placeholderTextColor: "#999999"
                color: "#1A2332"
                font.pixelSize: 13
                selectByMouse: true
                onTextChanged: filterHistory()

                background: Rectangle {
                    color: "#FFFFFF"
                    radius: 4
                    border.color: "#D1D5DB"
                    border.width: 1
                }
            }

            Button {
                text: "返回"
                font.pixelSize: 13
                onClicked: historyWindow.close()
            }
        }

        // ===== 中间：历史列表 =====
        Label {
            text: "历史记录 (" + filteredModel.count + ")"
            font.pixelSize: 12
            color: "#666666"
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            color: "#F5F7FA"
            radius: 6
            border.color: "#E2E8F0"
            border.width: 1

            ListView {
                id: historyListView
                anchors.fill: parent
                anchors.margins: 4
                model: filteredModel
                clip: true
                spacing: 2

                delegate: ItemDelegate {
                    width: historyListView.width
                    height: 36
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

                // 空状态
                Text {
                    anchors.centerIn: parent
                    text: "暂无历史记录"
                    color: "#6B7A8F"
                    visible: filteredModel.count === 0
                }

                // 滚动条
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }
        }

        // ===== 底部：详情展示区域 =====
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
                        text: "复制"
                        font.pixelSize: 11
                        enabled: detailName.text !== "选择上方的 API 查看详情"
                        onClicked: {
                            var full = detailName.text + "\n" +
                                       "需要的头文件：" + stripHtml(neededHead.text) + "\n" +
                                       "解释：" + stripHtml(detailDesc.text) + "\n" +
                                       "参数：" + stripHtml(detailParams.text) + "\n" +
                                       "示例：" + stripHtml(detailExample.text) + "\n" +
                                       "说明：" + stripHtml(detailDetail.text)
                            clipboradUtilsManager.setText(full)
                            console.log("复制内容:", full)
                        }
                    }
                }

                Text {
                    id: neededHead
                    text: "<b>需要的头文件:</b> 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailDesc
                    text: "<b>解释:</b> 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailParams
                    text: "<b>参数:</b> 暂无数据"
                    font.pixelSize: 11
                    color: "#475569"
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.RichText
                }

                Text {
                    id: detailExample
                    text: "<b>示例:</b> 暂无数据"
                    font.pixelSize: 12
                    color: "#1A2332"
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

    // 加载历史列表
    function load() {
        //清空模型
        historyModel.clear()
        filteredModel.clear()
        //请求数据
        AppCore.loadHistoryList()
    }

    //过滤历史记录
    function filterHistory() {
        var keyword = searchInput.text.trim().toLowerCase()
        filteredModel.clear()

        if (!keyword) {
            //没有关键词，显示全部
            for (var i = 0; i < historyModel.count; i++) {
                var item = historyModel.get(i)
                filteredModel.append(item)
            }
            return
        }

        //按 name 或 desc 模糊匹配
        for (var j = 0; j < historyModel.count; j++) {
            var data = historyModel.get(j)
            var nameMatch = data.name.toLowerCase().indexOf(keyword) !== -1
            var descMatch = data.desc.toLowerCase().indexOf(keyword) !== -1
            if (nameMatch || descMatch) {
                filteredModel.append(data)
            }
        }
    }

    //去除 HTML 标签（用于复制）
    function stripHtml(text) {
        return text.replace(/<[^>]*>/g, "")
    }

    //信号连接
    Component.onCompleted: {
        AppCore.historyListLoaded.connect(function(results) {
            historyModel.clear()
            for (var i = 0; i < results.length; i++) {
                var item = results[i]
                historyModel.append({
                    name: item.name || "未命名",
                    head: item.head || "",
                    desc: item.desc || "",
                    params: item.params || "",
                    example: item.example || "",
                    detail: item.detail || ""
                })
            }
            // 应用当前过滤（如果有）
            filterHistory()
        })
    }

}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "highlighter.h"
#include "assembler.h"
#include "simulator.h"
#include "optiondialog.h"
#include "pipeline_simulator.h"
#include "knobs.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

KnobManager knobs;
bool pipelineEnabled = false;
int numRows;
int cursorMemory = 0;
QList<QStringList> rows;

vector<int> instructionStage = {-1, -1, -1, -1, -1};
set<int> traceCounters;
vector<vector<int>> forwardingDetails;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Apply syntax highlighting to txtEditor
    new Highlighter(ui->txtEditor->document());

    // Initial Setup
    ui->tableMC->hide();
    ui->runButton->hide();
    ui->prevButton->hide();
    ui->stepButton->hide();
    ui->resetButton->hide();
    ui->btnRegistor->hide();
    ui->btnMemory->hide();
    ui->tableRegister->hide();
    ui->tableMemory->hide();
    ui->txtConsole->hide();

    ui->labelJumpTo->hide();
    ui->labelAddress->hide();
    ui->btnMemUp->hide();
    ui->btnMemDown->hide();
    ui->selectMemJump->hide();
    ui->txtMemAddress->hide();
    ui->btnGoToMem->hide();

    ui->tableRegister->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid transparent;"         // outer border can be transparent if you want margin effect
        "   margin: 5px;"                            // overall margin
        "}"
        "QTableWidget::item {"
        "   padding: 5px;"                           // leave some padding within cells
        "   border-bottom: 1px solid white;"         // white lines between rows
        "}"
        "QHeaderView::section {"
        "   Foreground-color: #f0f0f0;"               // a light header Foreground color
        "   padding: 5px;"
        "   border: 1px solid white;"                // white borders in header
        "}"
    );

    ui->tableMemory->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid transparent;"         // outer border can be transparent if you want margin effect
        "   margin: 5px;"                            // overall margin
        "}"
        "QTableWidget::item {"
        "   padding: 5px;"                           // leave some padding within cells
        "   border-bottom: 1px solid white;"         // white lines between rows
        "}"
        "QHeaderView::section {"
        "   padding: 5px;"
        "   border: 0px;"
        "   border-bottom: 1px solid white;"                // white borders in header
        "   font-weight: bold;"                      // make text bold
        "}"
    );

    // Register Setup
    ui->tableRegister->setColumnCount(2);
    ui->tableRegister->setRowCount(32);
    ui->tableRegister->verticalHeader()->setDefaultSectionSize(30);
    int totalWidth = ui->tableRegister->viewport()->width();
    totalWidth-=15;
    int col0Width = totalWidth / 5;
    int col1Width = totalWidth - col0Width;
    ui->tableRegister->setColumnWidth(0, col0Width);
    ui->tableRegister->setColumnWidth(1, col1Width);
    ui->tableRegister->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for(int i=0; i<32; i++){
        ui->tableRegister->setItem(i, 0, new QTableWidgetItem(QString("x%1").arg(i)));
        ui->tableRegister->setItem(i, 1, new QTableWidgetItem(QString("0x%1").arg(R[i], 8, 16, QChar('0')).toUpper()));
    }

    // Memory Setup
    ui->tableMemory->setColumnCount(5);
    ui->tableMemory->setRowCount(14);
    ui->tableMemory->verticalHeader()->setDefaultSectionSize(30);
    totalWidth = ui->tableMemory->viewport()->width();
    totalWidth-=15;
    col0Width = totalWidth / 3;
    col1Width = totalWidth / 6;
    ui->tableMemory->setColumnWidth(0, col0Width);
    ui->tableMemory->setColumnWidth(1, col1Width);
    ui->tableMemory->setColumnWidth(2, col1Width);
    ui->tableMemory->setColumnWidth(3, col1Width);
    ui->tableMemory->setColumnWidth(4, col1Width);
    ui->tableMemory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for(int i=0; i<14; i++){
        int currAddress = (cursorMemory+13-i)*4;
        ui->tableMemory->setItem(i, 0, new QTableWidgetItem(QString("0x%1").arg(currAddress, 8, 16, QChar('0')).toUpper()));
        ui->tableMemory->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(MEM[currAddress+3], 2, 16, QChar('0')).toUpper()));
        ui->tableMemory->setItem(i, 2, new QTableWidgetItem(QString("%1").arg(MEM[currAddress+2], 2, 16, QChar('0')).toUpper()));
        ui->tableMemory->setItem(i, 3, new QTableWidgetItem(QString("%1").arg(MEM[currAddress+1], 2, 16, QChar('0')).toUpper()));
        ui->tableMemory->setItem(i, 4, new QTableWidgetItem(QString("%1").arg(MEM[currAddress], 2, 16, QChar('0')).toUpper()));
    }

    // Select Box Setup
    ui->selectMemJump->addItem("-- Choose --"); // Placeholder
    ui->selectMemJump->setItemData(0, Qt::AlignCenter, Qt::TextAlignmentRole);
    ui->selectMemJump->setItemData(0, QVariant(0), Qt::UserRole - 1); // Disable selection

    // Add actual options
    ui->selectMemJump->addItem("Text");
    ui->selectMemJump->addItem("Data");
    ui->selectMemJump->addItem("Heap");
    ui->selectMemJump->addItem("Stack");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnSimulator_clicked()
{
    // Choose knobs
    QList<bool> knobList;

    OptionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        knobList = dialog.getOptions();
    } else {
        return; // User canceled
    }

    // Write the assembly code from the text editor to input.asm
    QString asmContent = ui->txtEditor->toPlainText();
    QFile asmFile("input.asm");
    if (!asmFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not write to input.asm");
        return;
    }
    QTextStream asmOut(&asmFile);
    asmOut << asmContent;
    asmFile.close();

    // Call the assembler function directly.
    // parseAssembly is defined in your assembler module (assembler.cpp) and declared in assembler.h.
    parseAssembly("input.asm", "output.mc");

    // Update knobs
    knobs.pipelining = knobList[0];
    knobs.dataForwarding = knobList[1];
    knobs.traceBranchUnit = knobList[2];

    // Read the generated output.mc file to display the machine code
    QFile outFile("output.mc");
    if (!outFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not open output.mc");
        return;
    }
    QTextStream outStream(&outFile);
    QString machineCode = outStream.readAll();
    outFile.close();

    // // Update the tableMC to show the machine code
    QStringList lines = machineCode.split('\n');
    rows.clear();
    for(int i=0; i<lines.size(); i++){
        QString line = lines[i];
        int commaIndex = line.indexOf(',');
        QString leftPart = line.left(commaIndex).trimmed();
        QString rightPart = line.mid(commaIndex+1).trimmed();
        QStringList leftTokens = leftPart.split(' ');
        QString item1 = leftTokens[0];
        QString item2 = leftTokens[1];
        if (rightPart == QString("EOF")){
            QString item3 = rightPart.trimmed();
            QStringList row;
            row << item1 << item2 << item3;
            rows.append(row);
            break;
        }
        QStringList rightTokens = rightPart.split('#');
        QString item3 = rightTokens[0].trimmed();

        QStringList row;
        row << item1 << item2 << item3;
        rows.append(row);
    }
    traceCounters.clear();

    // If pipeline NOT enabled, use PHASE 2
    pipelineEnabled = knobList[0];
    if (!pipelineEnabled){

    ui->tableMC->setColumnCount(3);

    QStringList headers;
    headers << "PC" << "Machine Code" << "Assembly Code";
    ui->tableMC->setHorizontalHeaderLabels(headers);

    ui->tableMC->setStyleSheet(
        "QTableWidget::item {"
        "  border: 0px;"
        "  border-bottom: 1px solid grey;"
        "}"
        "QHeaderView::section {"
        "  border: 0px;"
        "  border-bottom: 1px solid white;"
        "  font-weight: bold;"
        "  font-size: 12pt;"
        "}"
    );

    ui->tableMC->setColumnWidth(0, 220);
    ui->tableMC->setColumnWidth(1, 220);
    ui->tableMC->setColumnWidth(2, 220);

    numRows = rows.size();
    ui->tableMC->setRowCount(numRows);
    for(int i=0; i<numRows; i++){
        QTableWidgetItem *item1 = new QTableWidgetItem(rows[i][0]);
        QTableWidgetItem *item2 = new QTableWidgetItem(rows[i][1]);
        QTableWidgetItem *item3 = new QTableWidgetItem(rows[i][2]);

        item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
        item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);
        item3->setFlags(item3->flags() & ~Qt::ItemIsEditable);

        if (i * 4 == PC) {
            item1->setForeground(QColor(255, 143, 64));
            item2->setForeground(QColor(255, 143, 64));
            item3->setForeground(QColor(255, 143, 64));
        }
        else{
            item1->setForeground(QColor(255, 255, 255));
            item2->setForeground(QColor(255, 255, 255));
            item3->setForeground(QColor(255, 255, 255));
        }

        ui->tableMC->setItem(i, 0, item1);
        ui->tableMC->setItem(i, 1, item2);
        ui->tableMC->setItem(i, 2, item3);
    }

    // Hide the text editor and update button states
    ui->runButton->show();
    ui->prevButton->show();
    ui->stepButton->show();
    ui->resetButton->show();

    ui->txtEditor->hide();
    ui->tableMC->show();
    ui->btnEditor->setEnabled(true);
    ui->btnSimulator->setEnabled(false);

    // Load Program Memory
    cursorMemory = 0;
    ui->txtMemAddress->clear();
    load_program_memory();
    resetRiscvSim();

    // Register/Memory Display
    ui->btnRegistor->show();
    ui->btnMemory->show();
    ui->tableRegister->show();
    ui->tableMemory->hide();
    ui->txtConsole->show();

    MainWindow::updateRegisterDisplay();
    MainWindow::updateMemoryDisplay();

    ui->btnMemory->setEnabled(true);
    ui->btnRegistor->setEnabled(false);
    MainWindow::updateMCDisplay();
    MainWindow::updateCMDStates();
    MainWindow::updateConsoleDisplay();

    }

    // PIPELINE, PHASE 3
    else{
        ui->tableMC->setColumnCount(6);

        QStringList headers;
        headers << "INST" << "F" << "D" << "EX" << "MB" << "WB";
        ui->tableMC->setHorizontalHeaderLabels(headers);

        ui->tableMC->setStyleSheet(
            "QTableWidget::item {"
            "  border: 0px;"
            "  border-bottom: 1px solid grey;"
            "}"
            "QHeaderView::section {"
            "  border: 0px;"
            "  border-bottom: 1px solid white;"
            "  font-weight: bold;"
            "  font-size: 12pt;"
            "}"
        );

        ui->tableMC->setColumnWidth(0, 260);
        ui->tableMC->setColumnWidth(1, 80);
        ui->tableMC->setColumnWidth(2, 80);
        ui->tableMC->setColumnWidth(3, 80);
        ui->tableMC->setColumnWidth(4, 80);
        ui->tableMC->setColumnWidth(5, 80);

        numRows = rows.size();
        ui->tableMC->setRowCount(numRows);
        for(int i=0; i<numRows; i++){
            QTableWidgetItem *item = new QTableWidgetItem(rows[i][2]);

            QTableWidgetItem *item1 = new QTableWidgetItem();
            QTableWidgetItem *item2 = new QTableWidgetItem();
            QTableWidgetItem *item3 = new QTableWidgetItem();
            QTableWidgetItem *item4 = new QTableWidgetItem();
            QTableWidgetItem *item5 = new QTableWidgetItem();

            item1->setForeground(QColor(255, 143, 64));
            item2->setForeground(QColor(255, 143, 64));
            item3->setForeground(QColor(255, 143, 64));
            item4->setForeground(QColor(255, 143, 64));
            item5->setForeground(QColor(255, 143, 64));

            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
            item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);
            item3->setFlags(item3->flags() & ~Qt::ItemIsEditable);
            item4->setFlags(item4->flags() & ~Qt::ItemIsEditable);
            item5->setFlags(item5->flags() & ~Qt::ItemIsEditable);

            item1->setText((i*4==instructionStage[0]?"✔":(i*4==instructionStage[0]-0x10000000?"🛑":(i*4==instructionStage[0]-0x20000000?"🌊":""))));
            item2->setText((i*4==instructionStage[1]?"✔":(i*4==instructionStage[1]-0x10000000?"🛑":(i*4==instructionStage[1]-0x20000000?"🌊":""))));
            item3->setText((i*4==instructionStage[2]?"✔":(i*4==instructionStage[2]-0x10000000?"🛑":(i*4==instructionStage[2]-0x20000000?"🌊":""))));
            item4->setText((i*4==instructionStage[3]?"✔":(i*4==instructionStage[3]-0x10000000?"🛑":(i*4==instructionStage[3]-0x20000000?"🌊":""))));
            item5->setText((i*4==instructionStage[4]?"✔":(i*4==instructionStage[4]-0x10000000?"🛑":(i*4==instructionStage[4]-0x20000000?"🌊":""))));

            ui->tableMC->setItem(i, 0, item);
            ui->tableMC->setItem(i, 1, item1);
            ui->tableMC->setItem(i, 2, item2);
            ui->tableMC->setItem(i, 3, item3);
            ui->tableMC->setItem(i, 4, item4);
            ui->tableMC->setItem(i, 5, item5);
        }

        // Hide the text editor and update button states
        ui->runButton->show();
        ui->prevButton->show();
        ui->stepButton->show();
        ui->resetButton->show();

        ui->txtEditor->hide();
        ui->tableMC->show();
        ui->btnEditor->setEnabled(true);
        ui->btnSimulator->setEnabled(false);

        // Load Program Memory
        cursorMemory = 0;
        ui->txtMemAddress->clear();
        pipeline::load_program_memory("output.mc");
        pipeline::resetPipelinedSimulator();
        instructionStage = {-1, -1, -1, -1, -1};
        forwardingDetails = {};

        // Register/Memory Display
        ui->btnRegistor->show();
        ui->btnMemory->show();
        ui->tableRegister->show();
        ui->tableMemory->hide();
        ui->txtConsole->show();

        MainWindow::updateRegisterDisplay();
        MainWindow::updateMemoryDisplay();

        ui->btnMemory->setEnabled(true);
        ui->btnRegistor->setEnabled(false);
        MainWindow::updateMCDisplay();
        MainWindow::updateCMDStates();
        MainWindow::updateConsoleDisplay();
    }
}

void MainWindow::on_btnEditor_clicked()
{
    // Read the generated output.mc file to display the machine code
    QFile inFile("input.asm");
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not open input.asm");
        return;
    }
    QTextStream inStream(&inFile);
    QString assemblyCode = inStream.readAll();

    // Update the text editor to show the machine code
    ui->txtEditor->setPlainText(assemblyCode);
    ui->txtEditor->setReadOnly(false);

    inFile.close();

    ui->runButton->hide();
    ui->prevButton->hide();
    ui->stepButton->hide();
    ui->resetButton->hide();

    ui->txtEditor->show();
    ui->tableMC->hide();
    ui->btnEditor->setEnabled(false);
    ui->btnSimulator->setEnabled(true);

    // Hide Register/Memory Table
    ui->btnRegistor->hide();
    ui->btnMemory->hide();
    ui->tableRegister->hide();
    ui->tableMemory->hide();
    ui->txtConsole->hide();

    ui->labelJumpTo->hide();
    ui->labelAddress->hide();
    ui->btnMemUp->hide();
    ui->btnMemDown->hide();
    ui->selectMemJump->hide();
    ui->txtMemAddress->hide();
    ui->btnGoToMem->hide();
}

void MainWindow::on_txtEditor_textChanged()
{
    // Optionally, handle text changes here.
}

// Updates the button states: Run, Step, Next, Reset
void MainWindow::updateCMDStates(){
    if (!pipelineEnabled){
        if (PC == 0 && PC == (numRows-1)*4){
            ui->runButton->setEnabled(false);
            ui->stepButton->setEnabled(false);
            ui->prevButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
        }
        else if (PC == 0){
            ui->runButton->setEnabled(true);
            ui->stepButton->setEnabled(true);
            ui->prevButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
        }
        else if (PC == (numRows-1)*4){
            ui->runButton->setEnabled(false);
            ui->stepButton->setEnabled(false);
            ui->prevButton->setEnabled(true);
            ui->resetButton->setEnabled(true);
        }
        else{
            ui->runButton->setEnabled(true);
            ui->stepButton->setEnabled(true);
            ui->prevButton->setEnabled(true);
            ui->resetButton->setEnabled(true);
        }
    }
    else{
        if (pipeline::stopFetching && pipeline::isPipelineEmpty()){
            ui->runButton->setEnabled(false);
            ui->stepButton->setEnabled(false);
        }
        else{
            ui->runButton->setEnabled(true);
            ui->stepButton->setEnabled(true);
        }
        ui->prevButton->setEnabled(false);
        ui->resetButton->setEnabled(true);
    }
}

// Updates the current machine code line highlighter
void MainWindow::updateMCDisplay(){
    ui->tableMC->clearArrows();
    if (!pipelineEnabled){
        for(int i=0; i<numRows; i++){
            if (i * 4 == PC) {
                ui->tableMC->item(i,0)->setForeground(QColor(255, 143, 64));
                ui->tableMC->item(i,1)->setForeground(QColor(255, 143, 64));
                ui->tableMC->item(i,2)->setForeground(QColor(255, 143, 64));
            }
            else{
                ui->tableMC->item(i,0)->setForeground(QColor(255, 255, 255));
                ui->tableMC->item(i,1)->setForeground(QColor(255, 255, 255));
                ui->tableMC->item(i,2)->setForeground(QColor(255, 255, 255));
            }
        }
    }
    else{
        for(int i=0; i<numRows; i++){
            ui->tableMC->item(i,1)->setText((i*4==instructionStage[0]?"✔":(i*4==instructionStage[0]-0x10000000?"🛑":(i*4==instructionStage[0]-0x20000000?"🌊":""))));
            ui->tableMC->item(i,2)->setText((i*4==instructionStage[1]?"✔":(i*4==instructionStage[1]-0x10000000?"🛑":(i*4==instructionStage[1]-0x20000000?"🌊":""))));
            ui->tableMC->item(i,3)->setText((i*4==instructionStage[2]?"✔":(i*4==instructionStage[2]-0x10000000?"🛑":(i*4==instructionStage[2]-0x20000000?"🌊":""))));
            ui->tableMC->item(i,4)->setText((i*4==instructionStage[3]?"✔":(i*4==instructionStage[3]-0x10000000?"🛑":(i*4==instructionStage[3]-0x20000000?"🌊":""))));
            ui->tableMC->item(i,5)->setText((i*4==instructionStage[4]?"✔":(i*4==instructionStage[4]-0x10000000?"🛑":(i*4==instructionStage[4]-0x20000000?"🌊":""))));
        }
        // Clear existing arrows and add two distinct ones:
        for(vector<int>& a: forwardingDetails){
            ui->tableMC->addArrow(a[0], a[1], a[2], a[3]);
        }
    }
}

// Updates the console display which shows cycle log
void MainWindow::updateConsoleDisplay(){
    // Read the generated cycle_log.txt file to display on console
    QFile logFile("cycle_log.txt");
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Could not open cycle_log.txt");
        return;
    }
    QTextStream logStream(&logFile);
    QString cycle_log = logStream.readAll();
    logFile.close();

    if (pipelineEnabled){
        QFile logFile("stats.txt");
        if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "File Error", "Could not open stats.txt");
            return;
        }
        QTextStream logStream(&logFile);
        QString stats = logStream.readAll();
        logFile.close();

        cycle_log += stats;
    }

    ui->txtConsole->setPlainText(cycle_log);
}

// Helper function to update register table display from global R array.
void MainWindow::updateRegisterDisplay()
{
    for (int i = 0; i < 32; i++) {
        ui->tableRegister->item(i, 1)->setText(QString("0x%1")
                                                    .arg((pipelineEnabled?pipeline::R[i]:R[i]), 8, 16, QChar('0')).toUpper());
    }
}

void MainWindow::updateMemoryDisplay()
{
    for(int i=0; i<14; i++){
        int currAddress = (cursorMemory+13-i)*4;
        ui->tableMemory->item(i, 0)->setText(QString("0x%1").arg(currAddress, 8, 16, QChar('0')).toUpper());
        ui->tableMemory->item(i, 1)->setText(QString("%1").arg((pipelineEnabled?pipeline::MEM[currAddress+3]:MEM[currAddress+3]), 2, 16, QChar('0')).toUpper());
        ui->tableMemory->item(i, 2)->setText(QString("%1").arg((pipelineEnabled?pipeline::MEM[currAddress+2]:MEM[currAddress+2]), 2, 16, QChar('0')).toUpper());
        ui->tableMemory->item(i, 3)->setText(QString("%1").arg((pipelineEnabled?pipeline::MEM[currAddress+1]:MEM[currAddress+1]), 2, 16, QChar('0')).toUpper());
        ui->tableMemory->item(i, 4)->setText(QString("%1").arg((pipelineEnabled?pipeline::MEM[currAddress]:MEM[currAddress]), 2, 16, QChar('0')).toUpper());
    }
}

void MainWindow::on_runButton_clicked()
{
    if (!pipelineEnabled){
        while (PC != (numRows-1)*4){
            stepRiscvSim();
            if (traceCounters.find(PC) != traceCounters.end()) break;
        }
    }
    else{
        auto details = pipeline::runPipelinedSimulator(knobs, traceCounters);
        instructionStage = details.first;

        if (pipeline::stopFetching && pipeline::isPipelineEmpty()){

        map<uint32_t, vector<pair<bool, bool>>> PHT_table = details.second;

        int total = 0, correct = 0;

        // Count total entries for table rows
        int rowCount = 0;
        for (const auto& entry : PHT_table)
            rowCount += entry.second.size();

        // GUI Setup
        QDialog window;
        window.setWindowTitle("PHT Table Visualization");
        QVBoxLayout *layout = new QVBoxLayout(&window);

        // Create table widget
        QTableWidget *table = new QTableWidget(rowCount, 3);
        table->setHorizontalHeaderLabels({"PC", "Predicted", "Actual"});

        // Fill table
        int row = 0;
        for (const auto& [pc, vec] : PHT_table) {
            for (const auto& [pred, act] : vec) {
                QString pc_str = QString("0x%1").arg(pc, 0, 16);
                QString pred_str = pred ? "true" : "false";
                QString act_str = act ? "true" : "false";

                table->setItem(row, 0, new QTableWidgetItem(pc_str));
                table->setItem(row, 1, new QTableWidgetItem(pred_str));
                table->setItem(row, 2, new QTableWidgetItem(act_str));

                bool matched = (pred == act);
                if (matched) correct++;
                total++;

                // Set background color
                QColor color = matched ? QColor(144, 238, 144) : QColor(255, 99, 71); // green or red
                for (int col = 0; col < 3; col++) {
                    table->item(row, col)->setBackground(color);
                    table->item(row, col)->setForeground(QBrush(QColor(0, 0, 0))); // Set text color to black
                }

                row++;
            }
        }

        // Accuracy label
        double accuracy = total ? (double)correct / total * 100.0 : 0.0;
        QString acc_str = QString("Accuracy: %1%").arg(QString::number(accuracy, 'f', 2));
        QLabel *accLabel = new QLabel(acc_str);
        accLabel->setStyleSheet(
            "QLabel { "
            "background-color: #FFD700; "    // richer yellow (golden)
            "padding: 6px; "
            "font-weight: bold; "
            "font-size: 14px; "
            "border: 1px solid black; "
            "border-radius: 6px; "
            "color: black; "                 // Set text color to black for visibility
            "}"
            );
        layout->addWidget(table);
        layout->addWidget(accLabel);
        window.setLayout(layout);
        window.resize(400, 300);
        window.exec();
        }
    }
    MainWindow::updateRegisterDisplay();
    MainWindow::updateMemoryDisplay();
    MainWindow::updateMCDisplay();
    MainWindow::updateCMDStates();
    MainWindow::updateConsoleDisplay();
}

void MainWindow::on_stepButton_clicked()
{
    if (!pipelineEnabled){
        stepRiscvSim();
    }
    else{
        auto details = pipeline::stepPipelinedSimulator(knobs);
        instructionStage = details.first;
        forwardingDetails = details.second;
        if (pipeline::stopFetching && pipeline::isPipelineEmpty()){
            MainWindow::on_runButton_clicked();
            return;
        }
    }
    MainWindow::updateRegisterDisplay();
    MainWindow::updateMemoryDisplay();
    MainWindow::updateMCDisplay();
    MainWindow::updateCMDStates();
    MainWindow::updateConsoleDisplay();
}

void MainWindow::on_prevButton_clicked()
{
    prevRiscvSim();
    MainWindow::updateRegisterDisplay();
    MainWindow::updateMemoryDisplay();
    MainWindow::updateMCDisplay();
    MainWindow::updateCMDStates();
    MainWindow::updateConsoleDisplay();
}

void MainWindow::on_resetButton_clicked()
{
    traceCounters.clear();
    for(int i=0; i<numRows; i++){
        ui->tableMC->item(i, 0)->setText(rows[i][2]);
    }
    if (!pipelineEnabled){
        resetRiscvSim();
    }
    else{
        instructionStage = {-1, -1, -1, -1, -1};
        forwardingDetails = {};
        pipeline::resetPipelinedSimulator();
    }
    MainWindow::updateRegisterDisplay();
    MainWindow::updateMemoryDisplay();
    MainWindow::updateMCDisplay();
    MainWindow::updateCMDStates();
    MainWindow::updateConsoleDisplay();
}

void MainWindow::on_btnRegistor_clicked()
{
    ui->tableRegister->show();
    ui->tableMemory->hide();

    ui->labelJumpTo->hide();
    ui->labelAddress->hide();
    ui->btnMemUp->hide();
    ui->btnMemDown->hide();
    ui->selectMemJump->hide();
    ui->txtMemAddress->hide();
    ui->btnGoToMem->hide();

    ui->btnMemory->setEnabled(true);
    ui->btnRegistor->setEnabled(false);
}

void MainWindow::on_btnMemory_clicked()
{
    ui->tableRegister->hide();
    ui->tableMemory->show();

    ui->labelJumpTo->show();
    ui->labelAddress->show();
    ui->btnMemUp->show();
    ui->btnMemDown->show();
    ui->selectMemJump->show();
    ui->txtMemAddress->show();
    ui->btnGoToMem->show();

    ui->btnMemory->setEnabled(false);
    ui->btnRegistor->setEnabled(true);
}

void MainWindow::on_btnGoToMem_clicked()
{
    QString inputText = ui->txtMemAddress->text(); // Get input from txtMemAddress
    bool ok;
    int address;

    if (inputText.startsWith("0x", Qt::CaseInsensitive)) {
        // If input starts with "0x", treat it as a hexadecimal number
        address = inputText.toInt(&ok, 16);
    } else {
        // Otherwise, treat it as a decimal number
        address = inputText.toInt(&ok, 10);
    }

    if (ok) {
        cursorMemory = address/4;
        cursorMemory = max(cursorMemory, 0);
        cursorMemory = min(cursorMemory, 536870898);
        updateMemoryDisplay();
        qDebug() << "Address:" << address;  // Successfully converted
    } else {
        qDebug() << "Invalid input!";
    }

    ui->txtMemAddress->clear();
}

void MainWindow::on_txtMemAddress_returnPressed()
{
    MainWindow::on_btnGoToMem_clicked();
}

void MainWindow::on_selectMemJump_currentIndexChanged(int index)
{
    if (index == 1){
        cursorMemory = 0;
        updateMemoryDisplay();
        ui->selectMemJump->setCurrentIndex(0);
    }
    else if (index == 2){
        cursorMemory = 67108864;
        updateMemoryDisplay();
        ui->selectMemJump->setCurrentIndex(0);
    }
    else if (index == 3){
        cursorMemory = 67117056;
        updateMemoryDisplay();
        ui->selectMemJump->setCurrentIndex(0);
    }
    else if (index == 4){
        cursorMemory = 536870898;
        updateMemoryDisplay();
        ui->selectMemJump->setCurrentIndex(0);
    }
}


void MainWindow::on_btnMemUp_clicked()
{
    cursorMemory += 14;
    cursorMemory = min(cursorMemory, 536870898);
    updateMemoryDisplay();
}


void MainWindow::on_btnMemDown_clicked()
{
    cursorMemory -= 14;
    cursorMemory = max(cursorMemory, 0);
    updateMemoryDisplay();
}


void MainWindow::on_tableMC_cellClicked(int row, int column)
{
    column = 0;
    if (ui->tableMC->item(row, column)->text().startsWith("🔴")){
        traceCounters.erase(traceCounters.find(4*row));
        ui->tableMC->item(row, column)->setText(rows[row][2]);
    }
    else{
        traceCounters.insert(4*row);
        ui->tableMC->item(row, column)->setText("🔴 "+rows[row][2]);
    }
}

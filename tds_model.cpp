#include "tds_model.h"
#include "ui_tds_model.h"
#include <vector>
#include <iostream>
#include <QFileDialog>
#include <fstream>
#include <sstream>

TDSmodel::TDSmodel(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TDSmodel),
	compared(new float[STEPS]),
	dataMultiplier(1)
{
	for (int i = 0; i < STEPS; i++)
		compared[i] = NAN;
	ui->setupUi(this);
}

TDSmodel::~TDSmodel()
{
	delete ui;
	delete[] compared;
}

void TDSmodel::on_exitButton_clicked()
{
	close();
}

void TDSmodel::on_pushButton_clicked()
{
	dataMultiplier = 1;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Data"), "", tr("Txt file (*.txt *.csv)"));
	if (fileName.length()) {
		std::ifstream fileText(fileName.toUtf8().constData());
		std::string str((std::istreambuf_iterator<char>(fileText)), std::istreambuf_iterator<char>());
		std::stringstream stream(str);
		std::string line;
		std::getline(stream, line);
		unsigned int column = ui->columnEdit->text().toInt();
		if (line[0] != '-' && (line[0] > '0' || line[0] > '9'))
			std::getline(stream, line);
		int lineNum = 0;
		do {
			if (line.empty()) break;
			const char* parsing = line.c_str();
			auto readFloat = [&] (const char* reading) -> float {
				float sign = (*reading == '-') ? reading++, -1 : 1;
				float result = 0;
				for (; *reading >= '0' && *reading <= '9'; reading++)
					result = result * 10 + *reading - '0';
				if (*reading == '.' || *reading == ',') {
					reading++;
					for (float divisor = 0.1; *reading >= '0' && *reading <= '9'; reading++, divisor /= 10)
						result += divisor * (*reading - '0');
				}
				if (*reading == 'E' || *reading == 'e') {
					reading++;
					int exponent = 0;
					int exponentSign = (*reading == '-') ? reading++, -1 : 1;
					for (; *reading >= '0' && *reading <= '9'; reading++)
						exponent = exponent * 10 + *reading - '0';
					exponent *= exponentSign;
					while (exponent > 0) exponent--, result *= 10;
					while (exponent < 0) exponent++, result /= 10;
				}
				return sign * result;
			};
			float x = readFloat(parsing);
			for (unsigned int i = 0; i < column; i++, parsing++) {
				while (*parsing != '\t' && *parsing != 0) parsing++;
			}
			float y = readFloat(parsing);
			//std::cerr << "Found point " << x << " " << y << std::endl;
			if (lineNum <= x - 1) {
				while (lineNum < x) {
					compared[lineNum] = NAN;
					lineNum++;
				}
				compared[lineNum] = y;
				lineNum++;
			}// else std::cerr << "Unfitting\n";
		} while (std::getline(stream, line));
		while (lineNum < STEPS) {
			compared[lineNum] = NAN;
			lineNum++;
		}
	}
}

float TDSmodel::getMatch(const float* pressures) {
	float match = 0;
	int values = 1;
	for (unsigned int i = 0; i < STEPS; i++) {
		if (compared[i] != compared[i]) continue; // NaN
		float difference = compared[i] - pressures[i];
		match += difference * difference;
		values++;
	}
	return sqrt(match / values);
}

void TDSmodel::readSettings() {
	given_.activationEnergy = ui->activationEnergyEdit->text().toFloat();
	given_.width = ui->widthEdit->text().toFloat();
	given_.diffusion = ui->diffusionConstantEdit->text().toFloat() / (given_.width / WIDTH); // Precompute the diffusion per step
	given_.secsPerKelvin = 1 / ui->heatingEdit->text().toFloat();
	given_.rateConstant = ui->reactionConstantEdit->text().toFloat();
	given_.minDepth = ui->minDepthEdit->text().toFloat() / 100;
	float oldDataMultiplier = dataMultiplier;
	dataMultiplier = ui->multiplierEdit->text().toFloat();
	for (unsigned int i = 0; i < STEPS; i++) compared[i] *= dataMultiplier / oldDataMultiplier;
	if (given_.minDepth >= 1 || given_.minDepth < 0) {
		given_.minDepth = 0;
		ui->minDepthEdit->setText(QString("0"));
	}
	given_.maxDepth = ui->maxDepthEdit->text().toFloat() / 100;
	if (given_.maxDepth > 1 ||given_. maxDepth < 0 || given_.maxDepth <= given_.minDepth) {
		given_.maxDepth = 1;
		ui->maxDepthEdit->setText(QString("100"));
	}
	given_.quantity = ui->amountEdit->text().toFloat() / (given_.maxDepth - given_.minDepth) * 6.022140857e14; // Avogadro's constant for nanomoles
	given_.constantFactor = ui->constEdit->text().toFloat();
	given_.linearFactor = ui->linEdit->text().toFloat();
}

void TDSmodel::on_simulateButton_clicked()
{
	readSettings();
	plot();
}

void TDSmodel::on_fitButton_clicked()
{
	readSettings();

	std::cout << "Parametres:\n";
	std::cout << "Activation energy: " << given_.activationEnergy << std::endl;
	std::cout << "Width: " << given_.width << std::endl;
	std::cout << "Diffusion: " << given_.diffusion << std::endl;
	std::cout << "Rate Constant: " << given_.rateConstant << std::endl;
	std::cout << "Min depth: " << given_.minDepth << std::endl;
	std::cout << "Max depth: " << given_.maxDepth << std::endl;
	std::cout << "Quantity: " << given_.quantity << std::endl;
	std::cout << "Constant factor: " << given_.constantFactor << std::endl;
	std::cout << "Linear factor: " << given_.linearFactor << std::endl;
	std::cout << "Secs per Kelvin: " << given_.secsPerKelvin << std::endl;

	entry step;
	step.activationEnergy = 1 + ui->activationEnergyChangeEdit->text().toFloat();
	step.width = 1 + ui->widthChangeEdit->text().toFloat();
	step.diffusion = 1 + ui->diffusionConstantChangeEdit->text().toFloat();
	step.rateConstant = 1 + ui->reactionConstantChangeEdit->text().toFloat();
	step.minDepth = 1 + ui->minDepthChangeEdit->text().toFloat();
	step.maxDepth = 1 + ui->maxDepthChangeEdit->text().toFloat();
	step.quantity = 1 + ui->amountChangeEdit->text().toFloat();
	step.constantFactor = 1 + ui->constChangeEdit->text().toFloat();
	step.linearFactor = 1 + ui->linChangeEdit->text().toFloat();

	int stepsRemaining = ui->timesFitEdit->text().toInt();

	auto updateValue = [&] (float num, QLineEdit* to) -> void {
		std::stringstream stream;
		stream << num;
		to->setText(stream.str().c_str());
	};

	auto printStuff = [&] () -> void {
		if (ui->updateChangesCheckBox->isChecked()) {
			updateValue(step.activationEnergy - 1, ui->activationEnergyChangeEdit);
			updateValue(step.width - 1, ui->widthChangeEdit);
			updateValue(step.diffusion - 1, ui->diffusionConstantChangeEdit);
			updateValue(step.rateConstant - 1, ui->reactionConstantChangeEdit);
			updateValue(step.minDepth - 1, ui->minDepthChangeEdit);
			updateValue(step.maxDepth - 1, ui->maxDepthChangeEdit);
			updateValue(step.quantity - 1, ui->amountChangeEdit);
			updateValue(step.constantFactor - 1, ui->constChangeEdit);
			updateValue(step.linearFactor - 1, ui->linChangeEdit);
		}
		updateValue(given_.activationEnergy, ui->activationEnergyEdit);
		updateValue(given_.width, ui->widthEdit);
		updateValue(given_.diffusion * (given_.width / WIDTH), ui->diffusionConstantEdit);
		updateValue(given_.rateConstant, ui->reactionConstantEdit);
		updateValue(given_.minDepth * 100, ui->minDepthEdit);
		updateValue(given_.maxDepth * 100, ui->maxDepthEdit);
		updateValue(given_.quantity * (given_.maxDepth - given_.minDepth) / 6.022140857e14, ui->amountEdit);
		updateValue(given_.constantFactor, ui->constEdit);
		updateValue(given_.linearFactor, ui->linEdit);

		updateValue(stepsRemaining, ui->timesFitEdit);
		plot();
	};

	//std::chrono::high_resolution_clock::time_point lastPlotTime = std::chrono::high_resolution_clock::now();

	do {
		// Prepare values to test
		auto mutate = [&] (const float& mutating, const short int& mod, const short int& index) -> float {
			switch (mod) {
				case 1: return mutating * step.parametres[index];
				case 2: return mutating / step.parametres[index];
				default: return mutating;
			}
		};

		modEntry entry;
		given_.rateConstant = log(given_.rateConstant);
		for (int i = 0; i < PARAMETRES; i++) {
			entry.parametres[i][0] = mutate(given_.parametres[i], 0, i);
			entry.parametres[i][1] = mutate(given_.parametres[i], 1, i);
			entry.parametres[i][2] = mutate(given_.parametres[i], 2, i);
		}
		for (int i = PARAMETRES - 1; i < PARAMETRES + CONST_PARAMETRES; i++)
			for (int j = 0; j < 3; j++) {
				entry.parametres[i][j] = given_.parametres[i];
			}
		for (int i = 0; i < 3; i++)
			entry.parametres[3][i] = exp(entry.parametres[3][i]);

		// Compute
		std::cerr << "Computing " << VARIATIONS_SIZE << " variations " << getTimestamp() << std::endl;
		std::array<float, VARIATIONS_SIZE> computed = compute(entry, compared);
		std::cerr << "Done computing " << getTimestamp() << std::endl;

		int index = 0;
		float error = computed[0];
		int minPos = 0;
		unsigned short int touched[PARAMETRES];
		for (unsigned int i = 0; i < PARAMETRES; i++) touched[i] = 0;
		for (unsigned int i = 0; i < VARIATIONS_SIZE; i++) {
			//std::cerr << i << ": " << computed[i] << " vs " << computed[0] << std::endl;
			if (computed[i] < computed[0]) { // Skip the completely useless ones
				if (computed[i] < error) {
					error = computed[i];
					minPos = i;
				}
				index = i;
				for (int pos = PARAMETRES - 1; pos >= 0; pos--) {
					unsigned char order = index % 3;
					if (order == 1 || order == 2) touched[pos]++;
					index = (index - order) / 3;
				}
			}
		}
		std::cerr << "Found minimum at " << minPos << " " << computed[minPos] << std::endl;
		// Now, we simply commit the change
		index = minPos;
		for (int pos = PARAMETRES - 1; pos >= 0; pos--) {
			unsigned char order = index % 3;
			given_.parametres[pos] = entry.parametres[pos][order];
			index = (index - order) / 3;
		}
		std::cerr << "Mutated " << getTimestamp() << std::endl;
		bool makingSteps = false;
		std::cerr << "Updated changes " << getTimestamp() << std::endl;

		if (ui->updateChangesCheckBox->isChecked()) {
			for (unsigned int i = 0; i < PARAMETRES; i++) {
				// Increase those that participate in the improvement, reduce those that don't
				if (step.parametres[i] > 1.0001) makingSteps = true;
				if (touched[i] == 0) {
					if (step.parametres[i] > 1.0001) {
						makingSteps = true;
						step.parametres[i] = 0.25 + step.parametres[i] * 0.75;
					}
				} else {
					float importance = (float)touched[i] / (float)VARIATIONS_SIZE;
					step.parametres[i] = (1.0 - importance) * step.parametres[i] + step.parametres[i] * step.parametres[i] * importance;
					if (step.parametres[i] != step.parametres[i]) step.parametres[i] = 1.2;
				}
			}
		}
		stepsRemaining--;

//		std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();
//		std::chrono::nanoseconds timePassed = timeNow - lastPlotTime;
//		if (timePassed > std::chrono::nanoseconds(1000000000)) {
//			lastPlotTime = timeNow;

			printStuff();

			qApp->processEvents();
			std::cerr << "Updated numbers " << getTimestamp() << std::endl;

			if (!makingSteps) {
				std::cerr << "Not making steps, ending\n";
				break;
			}
//		}
	} while (stepsRemaining > 0);

	printStuff();

	if (stepsRemaining < 1) stepsRemaining = 1;
	updateValue(stepsRemaining, ui->timesFitEdit);
}

void TDSmodel::plot() {
	std::array<float, STEPS> values = computeCPU(given_);
	std::cerr << "Computed plot\n";
	ui->plot->clearGraphs();
	ui->plot->addGraph();
	ui->plot->graph(0)->setPen(QPen(QColor(100, 100, 255)));
	ui->plot->addGraph();
	ui->plot->graph(1)->setPen(QPen(QColor(100, 255, 100)));
	float maximum = 0;
	for (unsigned int i = 0; i < STEPS; i++) {
		if (values[i] == values[i]) {
			float val = values[i];
			ui->plot->graph(0)->addData(i, val);
			if (val > maximum) maximum = val;
		}
		if (compared[i] == compared[i]) ui->plot->graph(1)->addData(i, compared[i]);
		//std::cerr << "Plotting " << i << " " << compared[i] << std::endl;
	}
	std::stringstream matchStream;
	matchStream << "Match: " << getMatch(&(values[0]));
	std::string matchString = matchStream.str();
	ui->matchLabel->setText(QString(matchString.c_str()));
	ui->plot->xAxis->setRange(0, STEPS);
	ui->plot->xAxis->setLabel("Temperature [°C]");
	ui->plot->yAxis->setRange(0, maximum);
	ui->plot->yAxis->setLabel("Particles desorbed");
	ui->plot->replot();
	std::cerr << "Done\n";
}

void TDSmodel::on_resetChangeRangesButton_clicked()
{
	auto reset = [] (QLineEdit* widget) {
		widget->setText("0.1");
	};
	reset(ui->activationEnergyChangeEdit);
	reset(ui->diffusionConstantChangeEdit);
	reset(ui->amountChangeEdit);
	reset(ui->widthChangeEdit);
	reset(ui->minDepthChangeEdit);
	reset(ui->maxDepthChangeEdit);
	reset(ui->reactionConstantChangeEdit);
	reset(ui->constChangeEdit);
	reset(ui->linChangeEdit);
}

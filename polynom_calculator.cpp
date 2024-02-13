#include <iostream>
#include <initializer_list>
#include <vector>
#include <limits>
#include <cmath>
#include <cassert>
#include <stdexcept>

// Aufgabe 2.2

class Polynom {
public:
	// Polynome sollen uninitialisiert das Nullpolynom sein, also einen Koeffizienten = 0 haben.
	Polynom(): coeffs(1, 0.0) /* Ein Element mit Wert 0.0 */ {}
	
	// Erhalte den Grad des Polynoms zurück.
	int grad() const {
		return static_cast<int>(coeffs.size()) - 1;
	}
	
	// Erhalte den n-ten Koeffizienten des Polynoms zurück.
	double koeffizient(int n) const {
		assert(n >= 0);
		if (n > grad()) {
			return 0.0;
		}
		return coeffs[grad() - n];
	}
	
	// Werte das Polynom an der Stelle x aus.
	double auswerten(double const x) const {
		double y = 0.0;
		double z = 1.0;
		for (int n = 0; n <= grad(); ++n) {
			y += koeffizient(n) * z;
			z *= x;
		}
		return y;
	}
	
	// Andere Schreibweise für die Funktion 'auswerten()'.
	double operator()(double x) const { return auswerten(x); }
	
	// Setze den n-ten Koeffizienten des Polynoms auf den Wert x.
	void setKoeffizient(int n, double x) {
		assert(n >= 0);
		if (n > grad()) {
			coeffs.insert(coeffs.begin(), n - grad(), 0.0);
		}
		coeffs[grad() - n] = x;
		normalisieren();
	}
	
	// Lese ein Polynom über die Kommandozeile ein.
	void einlesen() {
		*this = Polynom();
		std::cout << "Geben Sie der Reihe nach die Koeffizienten ein. Geben Sie anschließend einen beliebigen Buchstaben ein.\n";
		for (int n = 0; ; ++n) {
			double c = 0.0;
			std::cout << "c_" << n << " = ";
			std::cin >> c;
			if (std::cin.fail()) {
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				return;
			}
			setKoeffizient(n, c);
		}
	}
	
	// Erhalte die Ableitung des Polynoms p zurück.
	friend Polynom ableiten(Polynom p) {
		if (p.grad() == 0) {
			return Polynom();
		}
		p.coeffs.pop_back();
		for (int i = 0; i < p.coeffs.size(); ++i) {
			p.coeffs[i] *= p.grad() - i + 1;
		}
		return p;
	}
	
	// Gebe das Polynoms p über das Streamobject str aus 
	friend std::ostream& operator<<(std::ostream& str, Polynom const& p) {
		int n = p.grad();
		for (double c: p.coeffs) {
			str << c << " * X^" << n--;
			if (n >= 0) {
				str << " + ";
			}
		}
		return str;
	}


	// Hilfsfunktion um obere Koeffizienten = 0 zu löschen.
private:
	void normalisieren() {
		while (coeffs[0] == 0.0 && grad() > 0) {
			coeffs.erase(coeffs.begin());
		}
	}
	
private:
	std::vector<double> coeffs;
};

// Fehlertoleranz für die Bestimmung der Nullstellen.
static double const eps = 1.0e-10;

// Funktion zur Bestimmung einer Nullstelle von p über das Newtonverfahren.
double bestimmeNullstelle(Polynom const& p, double startwert) {
	Polynom const q = ableiten(p);
	if (p.grad() == 0) {
		if (p.koeffizient(0) == 0.0) {
			return startwert;
		}
		throw std::runtime_error("p ist konstant und hat keine Nullstellen");
	}
	
	double x = startwert;
	
	int n = 0;
	while (true) {
		double const qx = q(x);
		// Vermeide Division durch 0
		if (std::abs(qx) < eps) {
			x += eps;
			continue;
		}
		double const px = p(x);
		double const nächstesX = x - px / qx;

		// Sobald die Iteration x nicht mehr stark verändert haben wir einen Fixpunkt also eine Nullstelle gefunden.
		if (std::abs(x - nächstesX) < eps) {
			break;
		}
		if (std::isinf(nächstesX)) {
			throw std::runtime_error("x == INF. Numerischer Fehler");
		}
		if (std::isnan(nächstesX)) {
			throw std::runtime_error("x == NAN. Numerischer Fehler");
		}

		x = nächstesX;

		// Mechanismus um eine nicht-terminierende Schleife zu vermeiden.
		int const maxIterationen = 1000000;
		if (++n > maxIterationen) {
			throw std::runtime_error("bestimmeNullstelle scheint nicht zu konvergieren. Möglicherweise hat p keine reellen Nullstellen oder die Iteration ist in einem Zyklus gefangen");
		}
	}
	
	return x;
}

// Teste ein Polynom p mit Start- und Referenzwert.
void testNullstellenBestimmung(Polynom const& p, double startwert, double referenz) {
	if (std::abs(bestimmeNullstelle(p, startwert) - referenz) >= eps) {
		std::cout << "p = " << p << " mit Startwert x = " << startwert << " besteht den Test nicht.\n";
	}
	else {
		std::cout << "Test bestanden.\n";
	}
}

// Teste die in der Aufgabe geforderten Polynome.
void testNullstellenBestimmung() {
	try {
		Polynom p;
		p.setKoeffizient(0, -1);
		p.setKoeffizient(2, 1);
		testNullstellenBestimmung(p, /* startwert = */  0.5, /* referenz = */  1.0);
		testNullstellenBestimmung(p, /* startwert = */ -0.5, /* referenz = */ -1.0);
		p.setKoeffizient(0, -5);
		testNullstellenBestimmung(p, /* startwert = */  1.0, /* referenz = */ std::sqrt(5));
	}
	catch (std::runtime_error const& e) {
		std::cerr << e.what() << std::endl;
	}
}

// Ausgabe 2.3

// Erhalte die Summe zweier Polynome zurück.
Polynom add(Polynom p, Polynom q) {
	if (q.grad() > p.grad()) {
		std::swap(p, q);
	}
	for (int n = 0; n <= q.grad(); ++n) {
		p.setKoeffizient(n, p.koeffizient(n) + q.koeffizient(n));
	}
	return p;
}



Polynom testpolynomErzeugen();

int main() {
	
	// 2.2
	testNullstellenBestimmung();
	
	// 2.3
	
	Polynom p;
	
	// Abkürzung für das Testpolynom
	std::cout << "Möchten sie das Testpolynom direkt zuweisen? (y = Ja, sonst = Nein)\n";
	bool testpolynomDirektZuweisen = false;
	{
		char auswahl;
		std::cin >> auswahl;
		if (std::cin.fail()) {
			std::cin.clear();
		}
		testpolynomDirektZuweisen = auswahl == 'y';
	}
	if (testpolynomDirektZuweisen) {
		p = testpolynomErzeugen();
	}
	else {
		std::cout << "Bitte geben sie ein Polynom ein.\n";
		p.einlesen();
	}
	
	std::cout << "p = " << p << std::endl;
	
	while (true) {
		std::cout << "Bitte geben sie einen Startwert ein.\n";
		double startwert = 0;
		std::cin >> startwert;
		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Fehlerhafter Startwert.\n";
			continue;
		}
		try {
			double nullstelle = bestimmeNullstelle(p, startwert);
			std::cout << "Die durch das Newton-Verfahren bestimmte Nullstelle ist x = " << nullstelle << ".\n";
			std::cout << "Gegenprobe: p(" << nullstelle << ") = " << p(nullstelle) << ".\n";
		}
		catch (std::runtime_error const& e) {
			std::cout << e.what() << std::endl;
		}
	}
}

Polynom testpolynomErzeugen() {
	Polynom p;
	
	p.setKoeffizient(20, 1.0);
	p.setKoeffizient(19, -210.0);
	p.setKoeffizient(18, 20615.0);
	p.setKoeffizient(17, -1256850.0);
	p.setKoeffizient(16, 53327946.0);
	p.setKoeffizient(15, -1672280820.0);
	p.setKoeffizient(14, 40171771630.0);
	p.setKoeffizient(13, -756111184500.0);
	p.setKoeffizient(12, 11310276995381.0);
	p.setKoeffizient(11, -135585182899530.0);
	p.setKoeffizient(10, 1307535010540395.0);
	p.setKoeffizient(9, -10142299865511450.0);
	p.setKoeffizient(8, 63030812099294896.0);
	p.setKoeffizient(7, -311333643161390640.0);
	p.setKoeffizient(6, 1206647803780373360.0);
	p.setKoeffizient(5, -3599979517947607200.0);
	p.setKoeffizient(4, 8037811822645051776.0);
	p.setKoeffizient(3, -12870931245150988800.0);
	p.setKoeffizient(2, 13803759753640704000.0);
	p.setKoeffizient(1, -8752948036761600000.0);
	p.setKoeffizient(0, 2432902008176640000.0);
	
	return p;
}

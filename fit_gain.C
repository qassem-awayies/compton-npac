#include <TGraphErrors.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMath.h>
#include <fstream>
#include <vector>
#include <iostream>

void fit_gain(const char* filename="gain.dat", int n=10) {
  std::vector<double> V, P;
  std::ifstream f(filename);
  if(!f.is_open()){ std::cerr<<"Cannot open "<<filename<<"\n"; return; }
  double v,p;
  while(f >> v >> p) V.push_back(v), P.push_back(p);
  if(V.empty()){ std::cerr<<"No data read\n"; return; }

  int N = V.size();
  TGraph *g = new TGraph(N);
  for(int i=0;i<N;i++) g->SetPoint(i, V[i], P[i]);

  // Use 'can' instead of 'c' for the TCanvas
  TCanvas *can = new TCanvas("can","Gain fit",800,600);
  g->SetTitle("Peak channel vs HV;HV (V);Peak channel");
  g->Draw("AP*");

  TF1 *fpl = new TF1("fpl","[0]*pow(x,[1])", 0.9*V.front(), 1.1*V.back());
  fpl->SetParameter(0, P.back()*pow(V.back(), -2));
  fpl->SetParameter(1, 7.0);
  g->Fit(fpl,"R");

  double a = fpl->GetParameter(0);
  double s = fpl->GetParameter(1);
  double a_err = fpl->GetParError(0);
  double s_err = fpl->GetParError(1);

  double k = s / double(n);
  double k_err = s_err / double(n);

  // rename physics constant 'c' to avoid conflict with TCanvas
  double cval = pow(a, 1.0/double(n));
  double c_err = cval * (a_err / (double(n) * a));

  std::cout << "\n=== Fit results ===\n";
  std::cout << "Fit: P(V) = a * V^s\n";
  std::cout << "a = " << a << " ± " << a_err << "\n";
  std::cout << "s = " << s << " ± " << s_err << " (s = k * n)\n";
  std::cout << "Assuming n = " << n << "\n";
  std::cout << "k = " << k << " ± " << k_err << "\n";
  std::cout << "c = a^(1/n) = " << cval << " ± " << c_err << "\n";
  std::cout << "=> delta(V) = c * V^k\n\n";

  std::cout << "Per-point values:\n";
  for(int i=0;i<N;i++){
    double delta_i = pow(P[i], 1.0/double(n));
    std::cout << "V="<<V[i]<<" V, P="<<P[i]<<" -> delta = "<<delta_i<<"\n";
  }

  can->SetLogx();
  can->SetLogy();
  g->Draw("AP");
  fpl->Draw("same");
}


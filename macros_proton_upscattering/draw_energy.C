void draw_energy()
{
    gStyle -> SetTitleFont(132,"");
    gStyle -> SetTitleFont(132,"x");
    gStyle -> SetTitleFont(132,"y");
    gStyle -> SetTitleFont(132,"z");
    gStyle -> SetNumberContours(99);
    gStyle -> SetPalette(kCool);
    //TColor::InvertPalette();

    const int tag = 2024120;
    const bool write_data = false;
    const bool read_data = !write_data;
    const bool draw_fit = false;

    const int dvz = 50;
    const int offsetvz = 0;
    const int numVertexBin = 350/dvz;
    const int ivz1 = 0;
    const int ivz2 = 6;

    const int scaleXN = 1;
    double binSize = 1;
    double eMin = 8.0;
    double eMax = 17.4;

    double energies[] = {8.5,9,9.5,10,10.5,11,11.5,12,12.5,13,13.5,14,14.5,15,15.5,16,16.5,17};
    //double energies[] = {8.5,9,9.5,10,10.5,11,11.5,12,12.5,13,13.5,14};
    //double energies[] = {17};
    //double energies[] = {14,13.5,13,12.5,12,11.5,11,10.5,10,9.5,9,8.5};
    //double energies[] = {14,13.5};
    auto energy_index = [](double energy) { return int(energy*2-16)-1; };

    auto make_hist = [](TH1* hist, int option=0) {
        hist -> SetStats(0);
        //hist -> GetXaxis() -> CenterTitle();
        //hist -> GetYaxis() -> CenterTitle();
        if (option==0) {
            hist -> GetXaxis() -> SetTitleSize(0.05);
            hist -> GetXaxis() -> SetLabelSize(0.04);
            hist -> GetXaxis() -> SetTitleOffset(1.1);
            hist -> GetXaxis() -> SetTitleFont(132);
            hist -> GetXaxis() -> SetLabelFont(132);

            hist -> GetYaxis() -> SetTitleSize(0.05);
            hist -> GetYaxis() -> SetLabelSize(0.04);
            hist -> GetYaxis() -> SetTitleOffset(1.1);
            hist -> GetYaxis() -> SetTitleFont(132);
            hist -> GetYaxis() -> SetLabelFont(132);

            hist -> GetZaxis() -> SetLabelFont(132);
        }
        if (option==1) {
            hist -> GetXaxis() -> SetTitleSize(0.06);
            hist -> GetXaxis() -> SetLabelSize(0.05);
            hist -> GetXaxis() -> SetTitleOffset(0.95);
            hist -> GetXaxis() -> SetTitleFont(132);
            hist -> GetXaxis() -> SetLabelFont(132);

            hist -> GetYaxis() -> SetTitleSize(0.06);
            hist -> GetYaxis() -> SetLabelSize(0.05);
            hist -> GetYaxis() -> SetTitleOffset(0.90);
            hist -> GetYaxis() -> SetTitleFont(132);
            hist -> GetYaxis() -> SetLabelFont(132);

            hist -> GetZaxis() -> SetLabelFont(132);
        }
        if (option==2) {
            hist -> GetXaxis() -> SetTitleSize(0.07);
            hist -> GetXaxis() -> SetLabelSize(0.06);
            hist -> GetXaxis() -> SetTitleOffset(1.2);
            hist -> GetXaxis() -> SetTitleFont(132);
            hist -> GetXaxis() -> SetLabelFont(132);

            hist -> GetYaxis() -> SetTitleSize(0.07);
            hist -> GetYaxis() -> SetLabelSize(0.06);
            hist -> GetYaxis() -> SetTitleOffset(1.2);
            hist -> GetYaxis() -> SetTitleFont(132);
            hist -> GetYaxis() -> SetLabelFont(132);

            hist -> GetZaxis() -> SetLabelFont(132);
        }
    };

    auto graphXNInput = new TGraph();
    ifstream file("dataset_carydavid_beardmary.txt");
    double x, y;
    while (file >> x >> y)
        graphXNInput -> SetPoint(graphXNInput->GetN(),x,y);

    auto win = LKWindowManager::GetWindowManager();

    auto histFull  = new TH2D("histFull", ";Proton energy at reaction (MeV);d#sigma/dE (mb/MeV)",1000,eMin,eMax,100,0,6*scaleXN/binSize);
    auto histGraph = new TH2D("histGraph",";Proton energy at reaction (MeV);Total cross-section (mb)",1000,eMin,eMax,100,0,100);

    make_hist(histFull);
    make_hist(histGraph,1);

    auto cvsGraph = win -> Canvas("cvs_graph");
    histGraph -> Draw();

    auto cvsFull = win -> Canvas("cvs_dsde");
    histFull -> Draw();

    TH1D *histEnergySum[30];
    int numOptions[30];
    TH1D *histED[30][10];
    //TGraphErrors *graphCrossSection[30][10];
    TGraph *graphCrossSection[30][10];

    auto hist_vz = new TH1D("hist_vz","",150,-50,400);
    auto hist_vz_e_all = new TH2D("hist_vz_e_all","",200,0,400,600,eMin,eMax);

    hist_vz -> SetStats(0);
    hist_vz_e_all -> SetStats(0);

    TFile* fileOut = nullptr;
    if (write_data)
        fileOut = new TFile(Form("histograms_%d.root",tag),"recreate");
    else
        fileOut = new TFile(Form("histograms_%d.root",tag),"read");

    TString fileName;
    TClonesArray *trackArray = nullptr;

    auto get_color = [](int i) {
        if (i==0) return int(kGray+2);
        else if (i==1) return int(kRed+1);
        else if (i==2) return int(kOrange-3);
        else if (i==3) return int(kSpring-1);
        else if (i==4) return int(kAzure+7);
        else if (i==5) return int(kBlue+1);
        else if (i==6) return int(kViolet);
        else if (i==7) return int(kCyan+2);
        else if (i==8) return int(kSpring-7);
        else if (i==9) return int(kPink+9);
        else if (i==10) return int(kMagenta+3);
        else if (i==11) return int(7);
        else if (i==12) return int(41);
        else if (i==13) return int(8);
        else if (i==14) return int(30);
        else if (i==15) return int(28);
        else if (i==16) return int(40);
        else if (i==18) return int(kTeal+1);
        //else if (i==17) return int(2);
        return int(kBlack);
    };

    for (auto beamEnergy : energies)
    {
        e_info << beamEnergy << " (" << energy_index(beamEnergy) << ")" << endl;
        auto iBeam = energy_index(beamEnergy);
        histEnergySum[iBeam] = new TH1D(Form("hist_e_sum%.2f",beamEnergy),"",1000,eMin,eMax);
        histEnergySum[iBeam]  -> SetLineColor(get_color(iBeam));
        //if (iBeam%2!=0) histEnergySum[iBeam] -> SetLineStyle(2);
        vector<TString> options;
        options.push_back("0.0");
        options.push_back("0.02");
        options.push_back("0.04");
        options.push_back("0.06");
        numOptions[iBeam] = options.size();
        TCanvas *cvsFit;
        if (draw_fit) {
            if (write_data) {
                cvsFit = win -> CanvasSquare(Form("cvs_fit_%d",iBeam),0.8);
                if      (options.size()>6) cvsFit -> Divide(3,3,0,0);
                else if (options.size()>4) cvsFit -> Divide(3,2,0,0);
                else                       cvsFit -> Divide(2,2,0,0);
            }
            else {
                cvsFit = win -> CanvasSquare(Form("cvs_fit_e_%d",iBeam),0.8);
                if      (options.size()>6) cvsFit -> Divide(3,3,0,0);
                else if (options.size()>4) cvsFit -> Divide(3,2,0,0);
                else                       cvsFit -> Divide(2,2,0,0);
            }
        }
        int countOption = 0;
        for (const char *thickness : options)
        {
            double vertexBinCount[10] = {0};
            auto hist = new TH1D(Form("hist_e_sum%.1f_d%s",beamEnergy,thickness),"",1000,eMin,eMax);
            histED[iBeam][countOption] = hist;
            //auto hist_vz_e = new TH2D(Form("hist_vz_e_%d_%d",iBeam,countOption),Form("degrader thickness = %s mm;Vertex position (mm);Proton energy at reaction (MeV)", thickness),200,-25,375,200,beamEnergy-1.0,beamEnergy+0.11);
            auto hist_vz_e = new TH2D(Form("hist_vz_e_%d_%d",iBeam,countOption),";Vertex position (mm);E_{proton} at reaction (MeV)",200,-25,375,200,beamEnergy-1.0,beamEnergy+0.06);
            hist_vz_e -> SetStats(0);
            auto hist_e = new TH1D(Form("hist_e_%d_%d",iBeam,countOption),";E - <E> at reaction (MeV)",200,-0.16,0.16);
            hist_e -> GetXaxis() -> SetNdivisions(505);
            hist_e -> GetYaxis() -> SetNdivisions(510);
            hist_e -> SetStats(0);

            TChain *chain = new TChain("event");
            //chain -> Add(Form("data/proton_sim_e%d_d%s.root",beamEnergy,thickness));
            auto nameFile = Form("data/proton_sim_%d_e%.1f_d%s.root",tag,beamEnergy,thickness);
            //e_info << nameFile << endl;
            chain -> Add(nameFile);
            chain -> SetBranchAddress("MCTrack",&trackArray);

            auto numEvents = chain -> GetEntries();

            TF1 *fit_vz_e = nullptr;
            if (read_data) {
                auto nameFit = Form("fit_%d_%d",iBeam,countOption);
                fit_vz_e = (TF1 *) fileOut -> Get(nameFit);
            }

            e_info << beamEnergy << " " << thickness << endl;
            double energy0 = 0;
            for (auto iEvent=0; iEvent<numEvents; ++iEvent)
            {
                chain -> GetEntry(iEvent);
                auto track = (LKMCTrack*) trackArray -> At(0);
                auto nv = track -> GetNumVertices();
                auto energyLast = track -> GetEnergy(nv-1);
                double escale = scaleXN*graphXNInput->Eval(energyLast);
                energy0 = track -> GetEnergy(0);
                hist -> Fill(energyLast,escale);
                auto vz = track -> GetVZ(nv-1);
                hist_vz -> Fill(vz);
                hist_vz_e -> Fill(vz,energyLast);
                hist_vz_e_all -> Fill(vz,energyLast);

                if (read_data) {
                    int iVertexBin = int((vz+offsetvz)/dvz);
                    if (iVertexBin>=0&&iVertexBin<numVertexBin) vertexBinCount[iVertexBin] += escale;
                    hist_e -> Fill(energyLast-fit_vz_e->Eval(vz));
                }
            }
            if (write_data) {
                fit_vz_e = new TF1(Form("fit_%d_%d",iBeam,countOption),"pol1",-50,400);
                hist_vz_e -> Fit(fit_vz_e,"N0Q");
                if (draw_fit) {
                    auto cvs2 = cvsFit -> cd(countOption+1);
                    double mgxy = 0.18;
                    double mgrt = 0.02;
                    //double mgrt = 0.18;
                    if (countOption==0) cvs2 -> SetMargin(mgxy,   0,   0,   mgrt);
                    if (countOption==1) cvs2 -> SetMargin(0,   mgrt,   0,   mgrt);
                    if (countOption==2) cvs2 -> SetMargin(mgxy,   0,   mgxy,   0);
                    if (countOption==3) cvs2 -> SetMargin(0,   mgrt,   mgxy,   0);
                    hist_vz_e -> Draw("col");
                    make_hist(hist_vz_e,2);
                    fit_vz_e -> SetLineColor(kBlack);
                    fit_vz_e -> Draw("samel");
                    fileOut -> cd();
                    fit_vz_e -> Write();
                }
            }
            else {
                if (draw_fit) {
                    TF1 *fit_e;
                    if (countOption==0)
                        fit_e = new TF1(Form("fit_e_%d_%d",iBeam,countOption),"gaus",-0.02,0.02);
                    else
                        fit_e = new TF1(Form("fit_e_%d_%d",iBeam,countOption),"gaus",-0.05,0.05);
                    hist_e -> Fit(fit_e,"N0Q");
                    auto cvs2 = cvsFit -> cd(countOption+1);
                    double mgxy = 0.18;
                    double mgrt = 0.02;
                    //double mgrt = 0.18;
                    double mgll = 0.18;
                    double mgrr = 0.02;
                    double mgbb = 0.18;
                    double mgtt = 0.02;
                    if (countOption==0) cvs2 -> SetMargin(mgxy,mgrr,   mgbb,mgrt);
                    if (countOption==1) cvs2 -> SetMargin(mgll,mgrt,   mgbb,mgrt);
                    if (countOption==2) cvs2 -> SetMargin(mgxy,mgrr,   mgxy,mgtt);
                    if (countOption==3) cvs2 -> SetMargin(mgll,mgrt,   mgxy,mgtt);
                    hist_e -> Draw();
                    make_hist(hist_e,2);
                    fit_e -> Draw("samel");
                    auto lgEFit = new TLegend();
                    lgEFit -> AddEntry((TObject*)nullptr,Form("%.3f",fit_e->GetParameter(2)/beamEnergy*100),"");
                    auto xIF1 = cvs2->GetUxmin() + cvs2->GetLeftMargin();
                    auto xIF2 = cvs2->GetUxmax() - cvs2->GetRightMargin();
                    auto yIF1 = cvs2->GetUymin() + cvs2->GetBottomMargin();
                    auto yIF2 = cvs2->GetUymax() - cvs2->GetTopMargin();
                    auto dx = xIF2 - xIF1;
                    auto dy = yIF2 - yIF1;
                    auto x1 = xIF1 + 0.65*dx;
                    auto x2 = xIF2 - 0.01;
                    auto y1 = yIF1 + 0.55*dy;
                    auto y2 = yIF2 - 0.01;
                    lgEFit -> SetX1(x1);
                    lgEFit -> SetX2(x2);
                    lgEFit -> SetY1(y1);
                    lgEFit -> SetY2(y2);
                    lgEFit -> SetBorderSize(0);
                    lgEFit -> SetTextFont(132);
                    lgEFit -> SetTextSize(0.050);
                    lgEFit -> SetMargin(0.45);
                    lgEFit -> SetNColumns(2);
                    lgEFit -> Draw();
                }

                graphCrossSection[iBeam][countOption] = new TGraph();
                auto graph = graphCrossSection[iBeam][countOption];
                graph -> SetMarkerStyle(20);
                graph -> SetMarkerSize(0.65);
                graph -> SetMarkerColor(get_color(iBeam));
                //graph -> SetLineColor(countOption+1);
                if (iBeam%2!=0) {
                    //graph -> SetMarkerStyle(25);
                    //graph -> SetMarkerSize(0.45);
                    //graph -> SetMarkerColor(kBlue);
                    //graph -> SetLineColor(kBlue);
                }
                for (auto ivz=ivz1; ivz<=ivz2; ++ivz)
                {
                    double vz1 = offsetvz + dvz*ivz;
                    double vz2 = offsetvz + dvz*(ivz+1);
                    auto dx0 = 0;//fit_vz_e->Eval(vz1) - fit_vz_e->Eval(vz2);
                    graph -> SetPoint(graph->GetN(), fit_vz_e -> Eval((vz1+vz2)/2.), vertexBinCount[ivz]/(100000/350*dvz));
                    //graph -> SetPointError(graph->GetN()-1, dx0, 0);
                }
            }
            hist -> Scale(1./100000/binSize);
            histEnergySum[iBeam] -> Add(hist);
            ++countOption;
        }
        if (draw_fit) {
            if (write_data)
                cvsFit -> SaveAs(Form("figures/figure_vz_e_%.1f.png",beamEnergy));
            else
                cvsFit -> SaveAs(Form("figures/figure_e_%.1f.png",beamEnergy));
        }
    }

    cvsFull -> cd();
    for (auto beamEnergy : energies) {
        auto iBeam = energy_index(beamEnergy);
        auto hist = histEnergySum[iBeam];
        if (iBeam%2!=0)  {
            //hist -> SetFillColor(hist->GetLineColor());
            //hist -> SetFillColor(hist->GetLineColor);
            hist -> SetLineColorAlpha(hist->GetLineColor(), 0.20);
            hist -> SetFillColorAlpha(hist->GetLineColor(), 0.20);
            hist -> Draw("samehistf");
        }
        else
            histEnergySum[iBeam] -> Draw("samehist");
    }

    auto lgFull = new TLegend();
    for (auto beamEnergy : energies)
        lgFull -> AddEntry(histEnergySum[energy_index(beamEnergy)],Form("%.1f MeV",beamEnergy),"f");
        //lgFull -> AddEntry(histEnergySum[energy_index(beamEnergy)],Form("%.1f",beamEnergy),"f");
    auto xIF1 = cvsFull->GetUxmin() + cvsFull->GetLeftMargin();
    auto xIF2 = cvsFull->GetUxmax() - cvsFull->GetRightMargin();
    auto yIF1 = cvsFull->GetUymin() + cvsFull->GetBottomMargin();
    auto yIF2 = cvsFull->GetUymax() - cvsFull->GetTopMargin();
    auto dx = xIF2 - xIF1;
    auto dy = yIF2 - yIF1;
    auto x1 = xIF1 + 0.30*dx;
    auto x2 = xIF2 - 0.01;
    auto y1 = yIF1 + 0.65*dy;
    auto y2 = yIF2 - 0.01;
    lgFull -> SetX1(x1);
    lgFull -> SetX2(x2);
    lgFull -> SetY1(y1);
    lgFull -> SetY2(y2);
    lgFull -> SetBorderSize(0);
    lgFull -> SetTextFont(132);
    lgFull -> SetTextSize(0.030);
    lgFull -> SetMargin(0.45);
    lgFull -> SetNColumns(4);
    lgFull -> Draw();
    cvsFull -> SaveAs("figures/figure_dsde.png");

    if (write_data) {
        fileOut -> cd();
        cvsFull -> Write();
        histFull -> Write();
        for (auto beamEnergy : energies)
            histEnergySum[energy_index(beamEnergy)] -> Write();
        for (auto beamEnergy : energies) {
            for (auto iOption=0; iOption<numOptions[energy_index(beamEnergy)]; ++iOption)
                histED[energy_index(beamEnergy)][iOption] -> Write();
        }
        lgFull -> Write();
    }
    else {
        cvsGraph -> cd();

        auto lgGraph = new TLegend();
        auto xIF1 = cvsGraph->GetUxmin() + cvsGraph->GetLeftMargin();
        auto xIF2 = cvsGraph->GetUxmax() - cvsGraph->GetRightMargin();
        auto yIF1 = cvsGraph->GetUymin() + cvsGraph->GetBottomMargin();
        auto yIF2 = cvsGraph->GetUymax() - cvsGraph->GetTopMargin();
        auto dx = xIF2 - xIF1;
        auto dy = yIF2 - yIF1;
        auto x1 = xIF1 + 0.45*dx;
        auto x2 = xIF2 - 0.01;
        auto y1 = yIF1 + 0.65*dy;
        auto y2 = yIF2 - 0.01;
        lgGraph -> SetX1(x1);
        lgGraph -> SetX2(x2);
        lgGraph -> SetY1(y1);
        lgGraph -> SetY2(y2);
        lgGraph -> SetBorderSize(0);
        lgGraph -> SetTextFont(132);
        lgGraph -> SetTextSize(0.030);
        lgGraph -> SetMargin(0.45);
        lgGraph -> SetNColumns(4);

        graphXNInput -> SetLineColorAlpha(kGray, 0.50);
        //graphXNInput -> SetLineColor(kGray);
        //graphXNInput -> SetLineWidth(5);
        graphXNInput -> Draw("samel");

        for (auto beamEnergy : energies) {
            auto graph2 = (TGraph*) graphCrossSection[energy_index(beamEnergy)][0] -> Clone();
            graph2 -> SetMarkerSize(1.2);
            lgGraph -> AddEntry(graph2,Form("%.1f MeV",beamEnergy),"p");
            for (auto iOption=0; iOption<numOptions[energy_index(beamEnergy)]; ++iOption) {
                graphCrossSection[energy_index(beamEnergy)][iOption] -> SetMarkerStyle(20);
                graphCrossSection[energy_index(beamEnergy)][iOption] -> Draw("samep");
            }
        }

        //lgGraph -> Draw();

        cvsGraph -> SaveAs("figures/figure_input_data.png");
    }
    return;

    win -> Canvas("cvs_bc");
    graphXNInput -> SetMarkerStyle(20);
    graphXNInput -> SetMarkerSize(0.5);
    graphXNInput -> Draw("apl");

    win -> Canvas("cvs_vz");
    hist_vz -> Draw();

    win -> Canvas("cvs_vz_e");
    hist_vz_e_all -> Draw("colz");
}

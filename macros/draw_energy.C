void draw_energy()
{
    auto histFull = new TH2D("histFull","Proton energy at reaction (MeV)",1000,7,25,100,0,20000);
    histFull -> SetStats(0);
    histFull -> GetXaxis() -> SetNdivisions(520);
    auto win = LKWindowManager::GetWindowManager();
    win -> Canvas("cvs");
    histFull -> Draw("colz");

    TString fileName;
    TClonesArray *trackArray = nullptr;
    //for (auto energy : {9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24})
    for (auto energy : {9,10,11,12})
    {
        TChain *chain = new TChain("event");
        auto hist = new TH1D(Form("hist_e%d",energy),"",1000,7,25);
        //for (const char *thickness : {"0.05","0.1","0.15","0.2"})
        //for (const char *thickness : {"0.05","0.1","0.15"})
        for (const char *thickness : {"0","0.02","0.04","0.08"})
            chain -> Add(Form("data/proton_sim_e%d_d%s.root",energy,thickness));
        chain -> SetBranchAddress("MCTrack",&trackArray);

        auto numEvents = chain -> GetEntries();
        cout << numEvents << endl;
        for (auto iEvent=0; iEvent<numEvents; ++iEvent)
        {
            chain -> GetEntry(iEvent);
            auto track = (LKMCTrack*) trackArray -> At(0);
            auto nv = track -> GetNumVertices();
            auto energy0 = track -> GetEnergy(0);
            auto energyLast = track -> GetEnergy(nv-1);
            hist -> Fill(energyLast);
        }
        if (energy%2==0)
            hist -> SetLineColor(kRed);
        hist -> Draw("same");
    }
}

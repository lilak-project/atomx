void test()
{
    auto run = new LKRun();
    run -> AddInputFile("data/tpc_sim.root");
    run -> Init();

    auto trackArray = run -> GetBranchA("MCTrack");
    run -> GetEntry(2);

    auto track = (LKMCTrack*) trackArray -> At(0);
    track -> Print();
}

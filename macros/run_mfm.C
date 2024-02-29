void run_mfm()
{
    auto run = new LKRun();

    run -> AddPar("config.mac");
    run -> SetEventTrigger(new LKMFMConversionTask());
    run -> Init();
    run -> SetEventCountForMessage(1);
    run -> Run();
}

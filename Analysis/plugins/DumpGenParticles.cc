#include "WtoJPsiDs/Analysis/plugins/DumpGenParticles.h"



DumpGenParticles::DumpGenParticles(const edm::ParameterSet& pSet):
  genParticlesToken_(consumes<reco::GenParticleCollection>(pSet.getUntrackedParameter<edm::InputTag>("genParticlesTag"))),
  verbosity_(pSet.getParameter<bool>("verbosity"))
{
  //---TFileService for output ntuples
  if( !fs_ )
  {
    throw edm::Exception(edm::errors::Configuration, "TFile Service is not registered in cfg file");
  }

  outTree_ = GenTree(pSet.getUntrackedParameter<std::string>("treeName").c_str(),"Gen tree for W > J/Psi D_s studies");
  entry_ = 0;
}


DumpGenParticles::~DumpGenParticles()
{}



void DumpGenParticles::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //---reset containers
  resonances_.clear();
  resonanceDaughters_.clear();
  resonanceDaughters2_.clear();
  
  
  //---reset output
  outTree_.Reset();
  outTree_.entry = entry_;
  outTree_.event = iEvent.id().event();
  outTree_.lumi = iEvent.id().luminosityBlock();
  outTree_.run = iEvent.id().run();
  
  
  //---load gen particles
  iEvent.getByToken(genParticlesToken_,genParticlesHandle_);
  auto genParticles = *genParticlesHandle_.product();
  
  for(size_t i = 0; i < genParticles.size(); ++i)
  {
    auto genParticle = genParticles[i];
    // {
    //   std::cout << &genParticle << std::endl;
    // }
    
    // save resonance
    if ( ( abs(genParticle.pdgId()) == 23 ||
           abs(genParticle.pdgId()) == 24 ||
           abs(genParticle.pdgId()) == 25 ||
           abs(genParticle.pdgId()) == 39 ||
           abs(genParticle.pdgId()) == 41 ||
           abs(genParticle.pdgId()) == 42 ) &&
         ( genParticle.status() == 22 || 
           genParticle.status() == 23 ) )
    {
      const reco::GenParticle* resonance = IsDecayed(&genParticle);
      resonances_.push_back( resonance );
      
      outTree_.reso_pt     -> push_back( resonance->pt() );
      outTree_.reso_eta    -> push_back( resonance->eta() );
      outTree_.reso_phi    -> push_back( resonance->phi() );
      outTree_.reso_energy -> push_back( resonance->energy() );
      outTree_.reso_mass   -> push_back( resonance->mass() );
      outTree_.reso_charge -> push_back( resonance->charge() );
      outTree_.reso_pdgId  -> push_back( resonance->pdgId() );
      outTree_.reso_name   -> push_back( GetParticleName(resonance->pdgId()) );
      
      std::vector<float> dau1_pt;
      std::vector<float> dau1_eta;
      std::vector<float> dau1_phi;
      std::vector<float> dau1_energy;
      std::vector<float> dau1_mass;
      std::vector<int> dau1_charge;
      std::vector<int> dau1_pdgId;
      std::vector<std::string> dau1_name;
      
      std::vector<int> dau2_n;
      std::vector<std::vector<float> > dau2_pt;
      std::vector<std::vector<float> > dau2_eta;
      std::vector<std::vector<float> > dau2_phi;
      std::vector<std::vector<float> > dau2_energy;
      std::vector<std::vector<float> > dau2_mass;
      std::vector<std::vector<int> > dau2_charge;
      std::vector<std::vector<int> > dau2_pdgId;
      std::vector<std::vector<std::string> > dau2_name;
      
      for(size_t ii = 0; ii < resonance->numberOfDaughters(); ++ii)
      {
        const reco::GenParticle* daughter = (const reco::GenParticle*)(resonance -> daughter(ii));
        resonanceDaughters_[resonance].push_back( daughter );
        
        dau1_pt.push_back( daughter->pt() );
        dau1_eta.push_back( daughter->eta() );
        dau1_phi.push_back( daughter->phi() );
        dau1_energy.push_back( daughter->energy() );
        dau1_mass.push_back( daughter->mass() );
        dau1_charge.push_back( daughter->charge() );
        dau1_pdgId.push_back( daughter->pdgId() );
        dau1_name.push_back( GetParticleName(daughter->pdgId()) );
        
        std::vector<float> temp_pt;
        std::vector<float> temp_eta;
        std::vector<float> temp_phi;
        std::vector<float> temp_energy;
        std::vector<float> temp_mass;
        std::vector<int> temp_charge;
        std::vector<int> temp_pdgId;
        std::vector<std::string> temp_name;
        
        for(size_t jj = 0; jj < daughter->numberOfDaughters(); ++jj)
        {
          const reco::GenParticle* daughter2 = (const reco::GenParticle*)(daughter -> daughter(jj));
          resonanceDaughters2_[resonance][daughter].push_back( daughter2 );
          
          temp_pt.push_back( daughter2->pt() );
          temp_eta.push_back( daughter2->eta() );
          temp_phi.push_back( daughter2->phi() );
          temp_energy.push_back( daughter2->energy() );
          temp_mass.push_back( daughter2->mass() );
          temp_charge.push_back( daughter2->charge() );
          temp_pdgId.push_back( daughter2->pdgId() );
          temp_name.push_back( GetParticleName(daughter2->pdgId()) );
        }
        
        dau2_n.push_back( daughter->numberOfDaughters() );
        dau2_pt.push_back( temp_pt );
        dau2_eta.push_back( temp_eta );
        dau2_phi.push_back( temp_phi );
        dau2_energy.push_back( temp_energy );
        dau2_mass.push_back( temp_mass );
        dau2_charge.push_back( temp_charge );
        dau2_pdgId.push_back( temp_pdgId );
        dau2_name.push_back( temp_name );
      }
      
      outTree_.resoDau1_n      -> push_back( resonance->numberOfDaughters() );
      outTree_.resoDau1_pt     -> push_back( dau1_pt );
      outTree_.resoDau1_eta    -> push_back( dau1_eta );
      outTree_.resoDau1_phi    -> push_back( dau1_phi );
      outTree_.resoDau1_energy -> push_back( dau1_energy );
      outTree_.resoDau1_mass   -> push_back( dau1_mass );
      outTree_.resoDau1_charge -> push_back( dau1_charge );
      outTree_.resoDau1_pdgId  -> push_back( dau1_pdgId );
      outTree_.resoDau1_name   -> push_back( dau1_name );
      
      outTree_.resoDau2_n      -> push_back( dau2_n );
      outTree_.resoDau2_pt     -> push_back( dau2_pt );
      outTree_.resoDau2_eta    -> push_back( dau2_eta );
      outTree_.resoDau2_phi    -> push_back( dau2_phi );
      outTree_.resoDau2_energy -> push_back( dau2_energy );
      outTree_.resoDau2_mass   -> push_back( dau2_mass );
      outTree_.resoDau2_charge -> push_back( dau2_charge );
      outTree_.resoDau2_pdgId  -> push_back( dau2_pdgId );
      outTree_.resoDau2_name   -> push_back( dau2_name );
    }
    
    outTree_.reso_n = resonances_.size();
    
    /*
    // save electrons
    if( abs(genParticle.pdgId()) == 11 )
    {
      const reco::GenParticle* Electron = &genParticle;
      selectedElectron->push_back(*Electron);
      selectedElectronPDGId->push_back(Electron->pdgId());
      selectedElectronTauDecay->push_back(0);
    }
    
    
    // save muons
    if( abs(genParticle.pdgId()) == 13 )
    {
      const reco::GenParticle* Muon = &genParticle;
      selectedMuon->push_back(*Muon);
      selectedMuonPDGId->push_back(Muon->pdgId());
      selectedMuonTauDecay->push_back(0);
    }
    
    
    // save top
    if (abs(genParticle.pdgId() ) == 6)
    {
      const reco::GenParticle* Top = TopFound(&genParticle);
      selectedTop->push_back(*Top);
      selectedTopPDGId->push_back(Top->pdgId());
      
      size_t topDaugthers = Top->numberOfDaughters();
      for(size_t ii = 0; ii< topDaugthers; ++ii)
      {
        if(abs(Top->daughter(ii)->pdgId())==5) 
        {
          selectedBQuark->push_back( *((reco::GenParticle*)Top->daughter(ii)) );
          selectedBQuarkPDGId->push_back(Top->daughter(ii)->pdgId());
        }        
      }
    }
    
    
    // save bosons
    if ( ( ( abs(genParticle.pdgId())==24 || abs(genParticle.pdgId())==23 ) && genParticle.status()==22 ) || ( abs(genParticle.pdgId())==25 && genParticle.status()==62 ) ) // needs to be checked if this workes for Z 23 as well
    {
      const reco::GenParticle * FinalBoson = BosonFound(&genParticle);
      selectedBoson->push_back(*FinalBoson);
      selectedBosonPDGId->push_back(FinalBoson->pdgId());
      
      int isLeptonicW = 0;
      size_t bosonDaugthers = FinalBoson->numberOfDaughters();
      for(size_t ii = 0; ii< bosonDaugthers; ++ii)
      {
        if (abs(FinalBoson->daughter(ii)->pdgId())== 11 || abs(FinalBoson->daughter(ii)->pdgId())== 13)
          isLeptonicW = 1;
        
        if(abs(FinalBoson->daughter(ii)->pdgId())== 11) 
        {
          selectedLepton->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLeptonPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
        }
        
        if(abs(FinalBoson->daughter(ii)->pdgId())== 13) 
        {
          selectedLepton->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLeptonPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
        }
        
        if(abs(FinalBoson->daughter(ii)->pdgId())== 15) 
        {
          selectedTau->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLepton->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLeptonPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
          
          //selectedTauHadTronic->push_back(0);
          const reco::GenParticle * FinalTauDecay = TauFound((reco::GenParticle*)FinalBoson->daughter(ii));
          int hadTauDecay=1;
          for(size_t iii=0; iii<FinalTauDecay->numberOfDaughters();iii++)
          {
            if(abs(FinalTauDecay->daughter(iii)->pdgId())== 11) 
            {
              selectedElectron->push_back( *((reco::GenParticle*)FinalTauDecay->daughter(iii)) );
              selectedElectronTauDecay->push_back(1);
              hadTauDecay=0;
            }
            if(abs(FinalTauDecay->daughter(iii)->pdgId())== 13) 
            {
              selectedMuon->push_back( *((reco::GenParticle*)FinalTauDecay->daughter(iii)) );
              selectedMuonTauDecay->push_back(1);
              hadTauDecay=0;
            }
          }
          selectedTauHadTronic->push_back(hadTauDecay);
        }
        
        if(abs(FinalBoson->daughter(ii)->pdgId())==12 || abs(FinalBoson->daughter(ii)->pdgId())==14 || abs(FinalBoson->daughter(ii)->pdgId())==16)
        {
          selectedNeutrino->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedNeutrinoPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
          selectedLepton->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLeptonPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
        }
        
        if(abs(FinalBoson->daughter(ii)->pdgId())>=1 && abs(FinalBoson->daughter(ii)->pdgId())<=6)
        {
          selectedQuark->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedQuarkPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
        }
        
        if(abs(FinalBoson->daughter(ii)->pdgId())== 22) 
        {
          selectedLepton->push_back( *((reco::GenParticle*)FinalBoson->daughter(ii)) );
          selectedLeptonPDGId->push_back(FinalBoson->daughter(ii)->pdgId());
        }
      }
      isSelectedBosonLeptonic->push_back(isLeptonicW);
    }
    */
  }
  
  if( verbosity_ )
  {
    std::cout << "--------------------------" << std::endl;
    /*
      for(unsigned int jj = 0; jj < selectedTop->size(); ++jj)
      {
      std::cout << ">>> TOP -- pdgId: " << std::fixed << std::setw(4) << (*selectedTop)[jj].pdgId()
      << "   status: " << std::fixed << std::setw(4) << (*selectedTop)[jj].status()
      << "   pT: "     << std::fixed << std::setprecision(2) << std::setw(8)  << (*selectedTop)[jj].pt()
      << "   eta: "    << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedTop)[jj].eta()
      << "   mass: "   << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedTop)[jj].mass()
      << std::endl;
      }
    */
    
    std::cout << ">>> RESONANCES" << std::endl;
    for(unsigned int ii = 0; ii < resonances_.size(); ++ii)
    {
      const reco::GenParticle* resonance = resonances_.at(ii);
      std::cout << resonance << std::endl;
      
      for(unsigned int jj = 0; jj < resonanceDaughters_[resonance].size(); ++jj)
      {
        const reco::GenParticle* daughter = (const reco::GenParticle*)(resonanceDaughters_[resonance].at(jj));
        std::cout << ">>>" << daughter << std::endl;
        
        for(unsigned int zz = 0; zz < (resonanceDaughters2_[resonance])[daughter].size(); ++zz)
        {
          const reco::GenParticle* daughter2 = (const reco::GenParticle*)((resonanceDaughters2_[resonance])[daughter].at(zz));
          std::cout << ">>>>>>" << daughter2 << std::endl;
        }
      }
    }
    std::cout << "<<< RESONANCES" << std::endl;
    
    /*
      for(unsigned int jj = 0; jj < selectedBoson->size(); ++jj)
      {
      std::cout << ">>>     BOSON -- pdgId: " << std::fixed << std::setw(4) << (*selectedBoson)[jj].pdgId()
      << "   status: " << std::fixed << std::setw(4) << (*selectedBoson)[jj].status()
      << "   pT: "     << std::fixed << std::setprecision(2) << std::setw(8)  << (*selectedBoson)[jj].pt()
      << "   eta: "    << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedBoson)[jj].eta()
      << "   mass: "   << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedBoson)[jj].mass()
      << std::endl;
      }
      for(unsigned int jj = 0; jj < selectedLepton->size(); ++jj)
      {
      std::cout << ">>>    LEPTON -- pdgId: " << std::fixed << std::setw(4) << (*selectedLepton)[jj].pdgId()
      << "   status: " << std::fixed << std::setw(4) << (*selectedLepton)[jj].status()
      << "   pT: "     << std::fixed << std::setprecision(2) << std::setw(8)  << (*selectedLepton)[jj].pt()
      << "   eta: "    << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedLepton)[jj].eta()
      << "   mass: "   << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedLepton)[jj].mass()
      << std::endl;
      }
      for(unsigned int jj = 0; jj < selectedQuark->size(); ++jj)
      {
      std::cout << ">>>     QUARK -- pdgId: " << std::fixed << std::setw(4) << (*selectedQuark)[jj].pdgId()
      << "   status: " << std::fixed << std::setw(4) << (*selectedQuark)[jj].status()
      << "   pT: "     << std::fixed << std::setprecision(2) << std::setw(8)  << (*selectedQuark)[jj].pt()
      << "   eta: "    << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedQuark)[jj].eta()
      << "   mass: "   << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedQuark)[jj].mass()
      << std::endl;
      }
      for(unsigned int jj = 0; jj < selectedBQuark->size(); ++jj)
      {
      std::cout << ">>>   B-QUARK -- pdgId: " << std::fixed << std::setw(4) << (*selectedBQuark)[jj].pdgId()
      << "   status: " << std::fixed << std::setw(4) << (*selectedBQuark)[jj].status()
      << "   pT: "     << std::fixed << std::setprecision(2) << std::setw(8)  << (*selectedBQuark)[jj].pt()
      << "   eta: "    << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedBQuark)[jj].eta()
      << "   mass: "   << std::fixed << std::setprecision(2) << std::setw(10) << (*selectedBQuark)[jj].mass()
      << std::endl;
      }
    */
    std::cout << "--------------------------" << std::endl;
  }
  
  
  outTree_.GetTTreePtr()->Fill();
  ++entry_;
}



const reco::GenParticle* DumpGenParticles::IsDecayed(const reco::GenParticle* particle)
{
  for(size_t i = 0; i< particle->numberOfDaughters(); ++i)
  {
    if( particle->daughter(i)->pdgId() == particle->pdgId() )
      return IsDecayed((reco::GenParticle*)particle->daughter(i));
  }
  
  return particle;
}


// const reco::GenParticle* DumpGenParticles::TopFound(const reco::GenParticle * particle)
// {
//   return particle;
// }


// const reco::GenParticle* DumpGenParticles::BosonFound(const reco::GenParticle * particle)
// {
//   for(size_t i=0;i< particle->numberOfDaughters();i++)
//   {
//     if(abs(particle->daughter(i)->pdgId())==24 || abs(particle->daughter(i)->pdgId())==23)
//       return BosonFound((reco::GenParticle*)particle->daughter(i));
//   }
//   return particle;
// }



// const reco::GenParticle* DumpGenParticles::TauFound(const reco::GenParticle * particle)
// {
//   for(size_t i=0;i< particle->numberOfDaughters();i++)
//   {
//     if(abs(particle->daughter(i)->pdgId())==24 || abs(particle->daughter(i)->pdgId())== 15)
//       return TauFound((reco::GenParticle*)particle->daughter(i));
//   }
//   return particle;
// }

std::ostream& operator<<(std::ostream& os, const reco::GenParticle* particle)
{
  os << ">>> pdgId: "      << std::fixed << std::setw(8) << particle->pdgId() << " ( " << std::fixed << std::setw(12) << GetParticleName(particle->pdgId()) << " )"
     << "        status: " << std::fixed << std::setw(2) << particle->status()
     << "        pT: "     << std::fixed << std::setprecision(3) << std::setw(6) << particle->pt()
     << "        eta: "    << std::fixed << std::setprecision(3) << std::setw(10) << particle->eta()
     << "        mass: "   << std::fixed << std::setprecision(3) << std::setw(6) << particle->mass();
  
  return os;
}

//define this as a plug-in
DEFINE_FWK_MODULE(DumpGenParticles);
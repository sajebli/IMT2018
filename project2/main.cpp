/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2005, 2006, 2007, 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/qldefines.hpp>
#ifdef BOOST_MSVC
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/batesengine.hpp>
#include <ql/pricingengines/vanilla/integralengine.hpp>
#include <ql/pricingengines/vanilla/fdeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdbermudanengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mcamericanengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

#include <ql/experimental/lattices/extendedbinomialtree.hpp>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif


int main(int, char* []) {

    try {

        boost::timer timer;
        std::cout << std::endl;

        // set up dates
        Calendar calendar = TARGET();
        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().evaluationDate() = todaysDate;

        // our options
        Option::Type type(Option::Put);
        Real underlying = 36;
        Real strike = 40;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;
        Date maturity(17, May, 1999);
        DayCounter dayCounter = Actual365Fixed();

        std::cout << "Option type = "  << type << std::endl;
        std::cout << "Maturity = "        << maturity << std::endl;
        std::cout << "Underlying price = "        << underlying << std::endl;
        std::cout << "Strike = "                  << strike << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;
        std::string method;
        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 45, 14, 14, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "European"
                  << std::setw(widths[2]) << std::left << "Bermudan"
                  << std::setw(widths[3]) << std::left << "American"
                  << std::endl;

        std::vector<Date> exerciseDates;
        for (Integer i=1; i<=4; i++)
            exerciseDates.push_back(settlementDate + 3*i*Months);

        ext::shared_ptr<Exercise> europeanExercise(
                                         new EuropeanExercise(maturity));

        ext::shared_ptr<Exercise> bermudanExercise(
                                         new BermudanExercise(exerciseDates));

        ext::shared_ptr<Exercise> americanExercise(
                                         new AmericanExercise(settlementDate,
                                                              maturity));

        Handle<Quote> underlyingH(
            ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

        // bootstrap the yield/dividend/vol curves
        Handle<YieldTermStructure> flatTermStructure(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));
        Handle<YieldTermStructure> flatDividendTS(
            ext::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            ext::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, calendar, volatility,
                                     dayCounter)));
        ext::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));
        ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
                 new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                               flatTermStructure, flatVolTS));

        // options
        VanillaOption europeanOption(payoff, europeanExercise);
        VanillaOption bermudanOption(payoff, bermudanExercise);
        VanillaOption americanOption(payoff, americanExercise);

        // Analytic formulas:

        // Black-Scholes for European
        // method = "Black-Scholes";
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //                              new AnalyticEuropeanEngine(bsmProcess)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // semi-analytic Heston for European
        // method = "Heston semi-analytic";
        // ext::shared_ptr<HestonProcess> hestonProcess(
        //     new HestonProcess(flatTermStructure, flatDividendTS,
        //                       underlyingH, volatility*volatility,
        //                       1.0, volatility*volatility, 0.001, 0.0));
        // ext::shared_ptr<HestonModel> hestonModel(
        //                                       new HestonModel(hestonProcess));
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //                              new AnalyticHestonEngine(hestonModel)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // semi-analytic Bates for European
        // method = "Bates semi-analytic";
        // ext::shared_ptr<BatesProcess> batesProcess(
        //     new BatesProcess(flatTermStructure, flatDividendTS,
        //                      underlyingH, volatility*volatility,
        //                      1.0, volatility*volatility, 0.001, 0.0,
        //                      1e-14, 1e-14, 1e-14));
        // ext::shared_ptr<BatesModel> batesModel(new BatesModel(batesProcess));
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //                                         new BatesEngine(batesModel)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // Barone-Adesi and Whaley approximation for American
        // method = "Barone-Adesi/Whaley";
        // americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //                new BaroneAdesiWhaleyApproximationEngine(bsmProcess)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << "N/A"
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;

        // Bjerksund and Stensland approximation for American
        // method = "Bjerksund/Stensland";
        // americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //               new BjerksundStenslandApproximationEngine(bsmProcess)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << "N/A"
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;

        // Integral
        // method = "Integral";
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //                                      new IntegralEngine(bsmProcess)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // Finite differences
        Size timeSteps = 5000;
        // method = "Finite differences";
        // europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDEuropeanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDBermudanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        //          new FDAmericanEngine<CrankNicolson>(bsmProcess,
        //                                              timeSteps,timeSteps-1)));
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << bermudanOption.NPV()
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;

        // Binomial method: Jarrow-Rudd
        method = "Extended Binomial Jarrow-Rudd";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedJarrowRudd>(bsmProcess,timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedJarrowRudd>(bsmProcess,timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedJarrowRudd>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;
        method = "Extended Binomial Cox-Ross-Rubinstein";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedCoxRossRubinstein>(bsmProcess,
                                                                   timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedCoxRossRubinstein>(bsmProcess,
                                                                   timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedCoxRossRubinstein>(bsmProcess,
                                                                   timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method: Additive equiprobabilities
        method = "Extended Additive equiprobabilities";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree>(bsmProcess,
                                                                   timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree>(bsmProcess,
                                                                   timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedAdditiveEQPBinomialTree>(bsmProcess,
                                                                   timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method: Binomial Trigeorgis
        method = "Extended Binomial Trigeorgis";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedTrigeorgis>(bsmProcess,timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedTrigeorgis>(bsmProcess,timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                new BinomialVanillaEngine<ExtendedTrigeorgis>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method: Binomial Tian
        method = "Extended Binomial Tian";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedTian>(bsmProcess,timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedTian>(bsmProcess,timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                      new BinomialVanillaEngine<ExtendedTian>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method: Binomial Leisen-Reimer
        method = "Extended Binomial Leisen-Reimer";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
              new BinomialVanillaEngine<ExtendedLeisenReimer>(bsmProcess,timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
              new BinomialVanillaEngine<ExtendedLeisenReimer>(bsmProcess,timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
              new BinomialVanillaEngine<ExtendedLeisenReimer>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method: Binomial Joshi
        method = "Extended Binomial Joshi";
        europeanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4>(bsmProcess,timeSteps)));
        bermudanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4>(bsmProcess,timeSteps)));
        americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
                    new BinomialVanillaEngine<ExtendedJoshi4>(bsmProcess,timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Monte Carlo Method: MC (crude)
        // timeSteps = 1;
        // method = "MC (crude)";
        // Size mcSeed = 42;
        // ext::shared_ptr<PricingEngine> mcengine1;
        // mcengine1 = MakeMCEuropeanEngine<PseudoRandom>(bsmProcess)
        //     .withSteps(timeSteps)
        //     .withAbsoluteTolerance(0.02)
        //     .withSeed(mcSeed);
        // europeanOption.setPricingEngine(mcengine1);
        // // Real errorEstimate = europeanOption.errorEstimate();
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // Monte Carlo Method: QMC (Sobol)
        // method = "QMC (Sobol)";
        // Size nSamples = 32768;  // 2^15
        //
        // ext::shared_ptr<PricingEngine> mcengine2;
        // mcengine2 = MakeMCEuropeanEngine<LowDiscrepancy>(bsmProcess)
        //     .withSteps(timeSteps)
        //     .withSamples(nSamples);
        // europeanOption.setPricingEngine(mcengine2);
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << europeanOption.NPV()
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << "N/A"
        //           << std::endl;

        // Monte Carlo Method: MC (Longstaff Schwartz)
        // method = "MC (Longstaff Schwartz)";
        // ext::shared_ptr<PricingEngine> mcengine3;
        // mcengine3 = MakeMCAmericanEngine<PseudoRandom>(bsmProcess)
        //     .withSteps(100)
        //     .withAntitheticVariate()
        //     .withCalibrationSamples(4096)
        //     .withAbsoluteTolerance(0.02)
        //     .withSeed(mcSeed);
        // americanOption.setPricingEngine(mcengine3);
        // std::cout << std::setw(widths[0]) << std::left << method
        //           << std::fixed
        //           << std::setw(widths[1]) << std::left << "N/A"
        //           << std::setw(widths[2]) << std::left << "N/A"
        //           << std::setw(widths[3]) << std::left << americanOption.NPV()
        //           << std::endl;

        // End test
        double seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nRun completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;
        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}

#!/usr/bin/env python3
# Generates synthetic, RAW (non-concentration-normalized) I(q) CSVs (US-SOMO
# SAXS load_iqq_csv wide-format) with 4 curves on an identical q grid at 4
# distinct concentrations.
#
# Raw measured/computed SAXS intensity is proportional to concentration to
# first order (more protein in the beam -> more scattered photons):
#   I(q,c) = c * I0_specific(q) * (1 + k*c)
# where I0_specific(q) is the (Guinier-shaped) intensity per mg/ml and the
# k*c term is a small structure-factor-like correction (inter-particle
# repulsion/attraction) causing a slight deviation from pure proportionality
# at higher concentration -- this is what do_extrap_c0's per-q linear fit
# (I = m*c + I0) is meant to characterize/remove. Because the data is
# genuinely proportional to c (not pre-normalized), the recovered intercept
# I0(q) at c=0 is expected to come out close to zero, with the linear term
# capturing I0_specific(q) -- that's the physically correct behavior for raw
# data and a legitimate thing to confirm with this fixture.
#
# Writes two files:
#   extrap_c0_test_iqq.csv          -- noiseless, exact ground truth
#   extrap_c0_test_iqq_poisson.csv  -- same ground truth with Poisson counting
#                                       noise (+ "I(q) sd" error rows), as is
#                                       appropriate for SAXS detector statistics
#                                       (shot noise on photon counts, not Gaussian
#                                       on I(q) directly)

import math
import random

Rg   = 25.0     # Angstrom
I0   = 100.0    # arbitrary units, specific intensity (per mg/ml) at q=0
k    = -0.02    # per mg/ml, small structure-factor-like nonlinearity in c

concentrations = [0.5, 1.0, 2.0, 4.0]  # mg/ml, 4 distinct values
names = [f"BSA_{c:g}mgml" for c in concentrations]

n_q = 41
q = [0.005 + i * (0.30 - 0.005) / (n_q - 1) for i in range(n_q)]

# counts(q,c) = I(q,c) * COUNT_SCALE -- the "incident flux / exposure" proxy
# that converts our arbitrary intensity units into photon counts for Poisson
# sampling. Chosen so the low-q points have good statistics (~1e5-1e6 counts)
# and statistics degrade naturally at high q as I(q) falls off, same as a
# real SAXS exposure.
COUNT_SCALE = 5000.0

random.seed( 20260621 )  # reproducible


def i0_specific( qv ):
    return I0 * math.exp( -( qv * qv * Rg * Rg ) / 3.0 )


def raw_I( qv, c ):
    return c * i0_specific( qv ) * ( 1.0 + k * c )


def poisson_sample( lam ):
    # Knuth's algorithm; fine for the lambda range used here (no noise needed
    # for performance, this is a one-off test fixture generator)
    if lam <= 0e0:
        return 0
    if lam > 500e0:
        # normal approximation is fine for these large counts and avoids
        # underflow in the multiplicative algorithm
        return max( 0, round( random.gauss( lam, math.sqrt( lam ) ) ) )
    l = math.exp( -lam )
    k_ = 0
    p = 1.0
    while True:
        k_ += 1
        p *= random.random()
        if p <= l:
            return k_ - 1


def write_csv( path, rows_I, rows_sd=None ):
    with open( path, "w" ) as f:
        q_csv = ",".join( f"{qv:.6g}" for qv in q )
        f.write( f'"Name","Type; q:",{q_csv},"Simulated test data"\n' )
        for i, name in enumerate( names ):
            i_csv = ",".join( f"{v:.6g}" for v in rows_I[ i ] )
            f.write( f'"{name}","I(q)",{i_csv},\n' )
            if rows_sd is not None:
                sd_csv = ",".join( f"{v:.6g}" for v in rows_sd[ i ] )
                f.write( f'"{name}","I(q) sd",{sd_csv},\n' )


# --- noiseless ground truth file ---

clean_rows = [ [ raw_I( qv, c ) for qv in q ] for c in concentrations ]
write_csv( "extrap_c0_test_iqq.csv", clean_rows )

# --- Poisson-noise file (counting statistics on photon counts, not Gaussian on I(q)) ---

noisy_rows = []
sd_rows    = []
for I in clean_rows:
    counts       = [ v * COUNT_SCALE for v in I ]
    noisy_counts = [ poisson_sample( c ) for c in counts ]
    noisy_rows.append( [ c / COUNT_SCALE for c in noisy_counts ] )
    # Poisson sd on counts is sqrt(counts); floor at 1 count to avoid a
    # literal zero error bar when zero photons were observed
    sd_rows.append( [ math.sqrt( max( c, 1 ) ) / COUNT_SCALE for c in noisy_counts ] )

write_csv( "extrap_c0_test_iqq_poisson.csv", noisy_rows, sd_rows )

print( f"wrote extrap_c0_test_iqq.csv and extrap_c0_test_iqq_poisson.csv: "
      f"{len(q)} q-points, {len(concentrations)} curves" )
print( "concentrations:", concentrations )
print( "raw I(q[0]={:.4g}) per concentration:".format( q[ 0 ] ),
      [ round( raw_I( q[ 0 ], c ), 4 ) for c in concentrations ] )
print( "i.e. I(0) scales with c (proportional, plus the small k*c structure-factor term)" )

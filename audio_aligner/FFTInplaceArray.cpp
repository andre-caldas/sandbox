#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <ranges>
#include <cassert>

#include "FFTInplaceArray.h"


FFTInplaceArray::FFTInplaceArray(void)
    : m_IsTransformed(false)
{
}

FFTInplaceArray::FFTInplaceArray(size_t minimum_size)
    : FFTInplaceArray()
{
    init(minimum_size);
}


void FFTInplaceArray::init(size_t minimum_size)
{
    m_MinimumSize = minimum_size;
    m_ActualComplexSize = compute_actual_complex_size(m_MinimumSize);

    m_FFT.reset(new FFTInplaceArray::PlanAndBuffer(m_ActualComplexSize));
}


FFTInplaceArray::~FFTInplaceArray(void)
{
}


int FFTInplaceArray::get_lag_from (FFTInplaceArray &from, double* quality)
{
    FFTInplaceArray correlation = correlate_with(from);

    int result = 0;
    double max = 0;

    for (size_t i = 0; i < correlation.m_ActualComplexSize; ++i)
    {
        double norm = std::norm(correlation.envelope(i));
        if (max < norm) {
            result = i;
            max = norm;
        }
    }

    if(nullptr != quality)
    {
        *quality = max;
    }

    // I don't know if this is correct.
    // But the transform is cyclic.
    // So, either we do something like this, or we double the buffer size.
    if (2*result > (int) m_ActualComplexSize)
    {
        result -= ((int)m_ActualComplexSize);
    }
    return result;
}


FFTInplaceArray FFTInplaceArray::clone(float drift)
{
    FFTInplaceArray result(m_MinimumSize);

    if(0.0 == drift)
    {
        std::memcpy(envelope(), result.envelope(), m_MinimumSize * sizeof(std::complex<double>));
    }
    else
    {
        float factor = 1.0 / (1.0 + drift);

        int shift = 0;
        for(size_t i = 0; i < m_MinimumSize; ++i)
        {
            int new_shift = std::round(factor * i) - i;

            while(new_shift > shift)
            {
                result.envelope(i+shift) = envelope(i);
                ++shift;
            }
            shift = new_shift;
            result.envelope(i+shift) = envelope(i);
        }
    }

    return result;
}


FFTInplaceArray& FFTInplaceArray::transform (void)
{
    if (!m_IsTransformed)
    {
        m_IsTransformed = true;
        m_FFT->execute();
    }

    return *this;
}


FFTInplaceArray& FFTInplaceArray::transform_back (void)
{
    if (m_IsTransformed)
    {
        m_IsTransformed = false;
        m_FFT->execute_inverse();
    }

    return *this;
}


FFTInplaceArray FFTInplaceArray::correlate_with (FFTInplaceArray &with)
{
    assert(with.m_MinimumSize == m_MinimumSize);

    FFTInplaceArray result(m_MinimumSize);
    return assign_correlate_with(result, with);
}


FFTInplaceArray& FFTInplaceArray::assign_correlate_with (FFTInplaceArray &assign_to, FFTInplaceArray &with)
{
    // Not really mandatory.
    assert(with.m_MinimumSize == m_MinimumSize);
    assert(assign_to.m_MinimumSize == m_MinimumSize);

    // Mandatory.
    assert(with.m_ActualComplexSize == m_ActualComplexSize);
    assert(assign_to.m_ActualComplexSize == m_ActualComplexSize);

    transform();
    with.transform();

    // Is it a good idea to use a FFTW_BACKWARD planner
    // instead of calculating the conjugate? Probably not...
    //
    // Wanted to use std::for_each with iota_view.
    // Either c++ is not very smart with ranges... like python,
    // or I am not, to figure how to do it.
    for(size_t i=0; i < m_ActualComplexSize; ++i)
    {
        assign_to.envelope(i) = envelope(i) * std::conj(with.envelope(i));
    }

    assign_to.m_IsTransformed = true;
    return assign_to.transform_back();
}


size_t FFTInplaceArray::compute_actual_complex_size (size_t minimum_size)
{
    // To always pad many zeros, so we don't get weird results... use result = 2.
    size_t result = 1;
    while(minimum_size > 0)
    {
        minimum_size = minimum_size >> 1;
        result = result << 1;
    }

    assert( result >= minimum_size );
    return result;
}


FFTInplaceArray::PlanAndBuffer::PlanAndBuffer (size_t size)
{
    // TODO: I do not know if `fftw_alloc_complex` is thread-safe.
    fftw_complex* buf = fftw_alloc_complex(size);
    m_Buffer = reinterpret_cast<std::complex<double>*>(buf);

    // TODO: This is Not thread safe unless we call
    // `fftw_make_planner_thread_safe()`.
    // However, I do not know if it is safe to call it many times
    // and in different threads.
    // So, by the time being, the application has to call it.
    m_Plan = fftw_plan_dft_1d(size, buf, buf, FFTW_FORWARD, FFTW_ESTIMATE);
    m_InversePlan = fftw_plan_dft_1d(size, buf, buf, FFTW_BACKWARD, FFTW_ESTIMATE);
    std::fill(//std::execution::par_unseq,
        m_Buffer,
        m_Buffer + size,
        std::complex<double>(0)
    );
}

FFTInplaceArray::PlanAndBuffer::~PlanAndBuffer(void)
{
    if (nullptr != m_Buffer)
    {
        fftw_free(reinterpret_cast<fftw_complex*>(m_Buffer));
        fftw_destroy_plan(m_Plan);
        fftw_destroy_plan(m_InversePlan);
    }
}


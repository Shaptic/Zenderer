#include "Zenderer/Objects/Animation.hpp"

using namespace zen;
using obj::zAnimation;

zAnimation::zAnimation(asset::zAssetManager& Assets) :
    zEntity(Assets), m_texc(0.0),
    m_framecount(0), m_current(0),
    m_now(0), m_rate(0),
    m_maxloops(0), m_loops(0),
    m_stop(false)
{
}

zAnimation::~zAnimation()
{
}

bool zAnimation::LoadFromTexture(const string_t& filename)
{
    gfx::zMaterial TmpMaterial(m_Assets);
    bool ret = TmpMaterial.LoadEffect(gfx::EffectType::SPRITESHEET) &&
               TmpMaterial.LoadTextureFromFile(filename);

    if(ret)
    {
        gfx::zQuad* pQ = new gfx::zQuad(m_Assets, m_Size.x, m_Size.y);
        pQ->AttachMaterial(TmpMaterial);
        pQ->Create();
        mp_allPrims.push_back(pQ);

        // Normalize texture coordinates.
        m_texc = 1.0 / (TmpMaterial.GetTexture().GetWidth() / m_Size.x);

        // Set shader parameters (these are done once).
        gfx::zEffect& e = const_cast<gfx::zEffect&>(pQ->GetMaterial().GetEffect());
        e.Enable();
        e.SetParameter("tc_offset", &m_texc, 1);
        e.SetParameter("proj", gfxcore::zRenderer::GetProjectionMatrix());
        e.Disable();

        // For collision.
        m_Box = math::aabb_t(math::rect_t(this->GetX(), this->GetY(),
                                          m_Size.x, m_Size.y));
    }

    return ret;
}

bool zAnimation::Update()
{
    // If we aren't enabled, or we aren't ready to animate, return.
    if(m_stop || ++m_now < m_rate) return false;

    // If we've reached the end of the animation cycle,
    // increase the loop count and start over.
    if(++m_current >= m_framecount)
    {
        ++m_loops;
        m_current = 0;
    }

    // If looping is limited (0 if not), and we've reached
    // the maximum number of loops, stop animating and
    // call our callback (if any).
    if(m_maxloops > 0 && m_loops >= m_maxloops)
    {
        m_maxloops = 0;
        this->StopAnimation(0);
        m_callback();
        return false;
    }

    // Next frame, start counter over for next animation.
    this->SwitchFrame(m_current);
    m_now = 0;
    return true;
}

void zAnimation::SetKeyframeCount(const uint16_t frames)
{
    m_framecount = frames;
}

void zAnimation::SetKeyframeSize(const math::vectoru16_t& Size)
{
    m_Size = Size;
}

void zAnimation::SetKeyframeRate(const uint16_t rate)
{
    m_rate = rate;
}

void zAnimation::PlayAnimation(const uint32_t loops /*= 0*/)
{
    this->SwitchFrame(0);
    m_maxloops  = loops;
    m_loops     = 0;
    m_stop      = false;
}

void zAnimation::StopAnimation(const int16_t frame /*= -1*/)
{
    this->SwitchFrame(frame < 0 ? m_current : frame);
    m_stop = true;
}

void zAnimation::OnFinish(std::function<void()>&& callback)
{
    m_callback = callback;
}

void zAnimation::SwitchFrame(const uint16_t frame)
{
    m_current = frame;
    real_t start[2] = { m_current * m_texc, 0.0 };

    gfx::zEffect& e = const_cast<gfx::zMaterial&>(
        mp_allPrims.front()->GetMaterial()
    ).GetEffect();

    e.Enable();
    e.SetParameter("tc_start", start, 2);
    e.Disable();
}

#include <GameUpdater.hxx>

GameUpdater::GameUpdater(Tube tube)
    : tube_(tube) {

}

static void doCollision(Ball & a, Ball & b) {
    cv::Point3f diff;
    diff.x = a.position.x - b.position.x;
    diff.y = a.position.y - b.position.y;
    diff.z = a.position.z - b.position.z;
    float norm = cv::norm(diff);
    diff.x /= norm;
    diff.y /= norm;
    diff.z /= norm;
    float dota = a.velocity.x * diff.x
        + a.velocity.y * diff.y
        + a.velocity.z * diff.z;
    float dotb = b.velocity.x * diff.x
        + b.velocity.y * diff.y
        + b.velocity.z * diff.z;
    float dotdiff = dota - dotb,
          cx = dotdiff * diff.x,
          cy = dotdiff * diff.y,
          cz = dotdiff * diff.z;
    a.velocity.x -= cx;
    a.velocity.y -= cy;
    a.velocity.z -= cz;
    b.velocity.x += cx;
    b.velocity.y += cy;
    b.velocity.z += cz;
}

void GameUpdater::tick(double dt, Balls & balls) const {
    for (Balls::iterator it = balls.begin();
            it != balls.end(); ++it) {
        it->second.position.x += it->second.velocity.x * dt;
        it->second.position.y += it->second.velocity.y * dt;
        it->second.position.z += it->second.velocity.z * dt;
        if (it->second.position.y < tube_.goal) {
            Balls::iterator del = it;
            --it;
            balls.erase(del);
            std::cout << "\a" << std::flush;
            continue;
        }
        if (std::abs(it->second.position.x) >= tube_.halfSize.width) {
            it->second.velocity.x *= -1;
        }
        if (std::abs(it->second.position.z) >= tube_.halfSize.height) {
            it->second.velocity.z *= -1;
        }
    }
    for (Balls::iterator a = balls.begin();
            a != balls.end(); ++a) {
        for (Balls::iterator b = balls.begin();
                b != a; ++b) {
            if (cv::norm(a->second.position - b->second.position) <= .25f) {
                doCollision(a->second, b->second);
            }
        }
    }
}
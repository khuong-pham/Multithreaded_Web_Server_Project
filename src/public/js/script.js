// Interactive JavaScript for Multithreaded Web Server
console.log('🚀 Multithreaded Web Server - JavaScript Module Loaded');

// Server information
const serverInfo = {
    startTime: Date.now(),
    requestCount: 0,
    features: {
        tcpSockets: true,
        httpParser: true,
        staticFiles: true,
        multithreading: false,
        caching: false,
        compression: false
    }
};

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', function() {
    console.log('🎯 Initializing web server dashboard...');
    
    initializeFeatures();
    updateServerInfo();
    addInteractiveElements();
    startStatusUpdates();
    
    console.log('✅ Dashboard initialization complete');
});

// Initialize interactive features
function initializeFeatures() {
    // Add click effects to feature cards
    const featureCards = document.querySelectorAll('.feature-card');
    featureCards.forEach((card, index) => {
        card.addEventListener('click', function() {
            animateCard(this);
            showFeatureDetails(this);
        });
        
        // Stagger the initial animation
        setTimeout(() => {
            card.style.opacity = '1';
            card.style.transform = 'translateY(0)';
        }, index * 100);
    });
    
    // Add hover effects to tech badges
    const techBadges = document.querySelectorAll('.tech-badge');
    techBadges.forEach(badge => {
        badge.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-3px) scale(1.05)';
        });
        
        badge.addEventListener('mouseleave', function() {
            this.style.transform = 'translateY(0) scale(1)';
        });
    });
    
    console.log(`📋 Initialized ${featureCards.length} feature cards`);
}

// Animate feature card on click
function animateCard(card) {
    card.style.transform = 'scale(1.02)';
    card.style.boxShadow = '0 15px 40px rgba(0,0,0,0.2)';
    
    setTimeout(() => {
        card.style.transform = '';
        card.style.boxShadow = '';
    }, 200);
}

// Show feature details
function showFeatureDetails(card) {
    const featureName = card.querySelector('h3').textContent;
    const status = card.querySelector('.status-badge').textContent;
    
    showNotification(`Feature: ${featureName} | Status: ${status}`, 'info');
}

// Update server information
function updateServerInfo() {
    // Update port number from current URL
    const port = window.location.port || '80';
    const portElement = document.getElementById('server-port');
    if (portElement) {
        portElement.textContent = port;
    }
    
    // Calculate uptime
    const uptime = Date.now() - serverInfo.startTime;
    console.log(`📊 Server uptime: ${formatUptime(uptime)}`);
    
    // Update page load performance
    if (performance && performance.timing) {
        const loadTime = performance.timing.loadEventEnd - performance.timing.navigationStart;
        console.log(`⚡ Page load time: ${loadTime}ms`);
    }
}

// Format uptime in human readable format
function formatUptime(milliseconds) {
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    
    if (hours > 0) return `${hours}h ${minutes % 60}m`;
    if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
    return `${seconds}s`;
}

// Add interactive elements
function addInteractiveElements() {
    // Add smooth scrolling for navigation
    const navLinks = document.querySelectorAll('.nav-link');
    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            if (this.getAttribute('href').startsWith('#')) {
                e.preventDefault();
                const target = document.querySelector(this.getAttribute('href'));
                if (target) {
                    target.scrollIntoView({ behavior: 'smooth' });
                }
            }
        });
    });
    
    // Add parallax effect to hero section
    window.addEventListener('scroll', function() {
        const scrolled = window.pageYOffset;
        const heroContent = document.querySelector('.hero-content');
        if (heroContent) {
            heroContent.style.transform = `translateY(${scrolled * 0.2}px)`;
        }
    });
    
    // Add typing effect to hero title
    addTypingEffect();
}

// Add typing effect to hero title
function addTypingEffect() {
    const heroTitle = document.querySelector('.hero-title');
    if (!heroTitle) return;
    
    const originalText = heroTitle.innerHTML;
    heroTitle.innerHTML = '<span class="emoji">🚀</span><span class="typed-text"></span><span class="version">v1.0</span>';
    
    const typedElement = heroTitle.querySelector('.typed-text');
    const textToType = " Multithreaded Web Server";
    let index = 0;
    
    function typeCharacter() {
        if (index < textToType.length) {
            typedElement.textContent += textToType.charAt(index);
            index++;
            setTimeout(typeCharacter, 100);
        }
    }
    
    setTimeout(typeCharacter, 1000);
}

// Test endpoint functionality
function testEndpoint(endpoint) {
    const resultsDiv = document.getElementById('test-results');
    if (!resultsDiv) return;
    
    const startTime = Date.now();
    resultsDiv.innerHTML = `🧪 Testing endpoint: ${endpoint}\n📡 Sending request...`;
    
    fetch(endpoint)
        .then(response => {
            const responseTime = Date.now() - startTime;
            const status = response.status;
            const statusText = response.statusText;
            
            let result = `📡 GET ${endpoint}\n`;
            result += `📊 Status: ${status} ${statusText}\n`;
            result += `⏱️ Response Time: ${responseTime}ms\n`;
            result += `📋 Headers:\n`;
            
            // Show some response headers
            for (let [key, value] of response.headers) {
                if (['content-type', 'content-length', 'server'].includes(key.toLowerCase())) {
                    result += `   ${key}: ${value}\n`;
                }
            }
            
            resultsDiv.innerHTML = result;
            
            // Show success/error notification
            if (status >= 200 && status < 300) {
                showNotification(`✅ ${endpoint} responded successfully (${responseTime}ms)`, 'success');
            } else {
                showNotification(`⚠️ ${endpoint} returned ${status} ${statusText}`, 'warning');
            }
            
            serverInfo.requestCount++;
            
        })
        .catch(error => {
            const responseTime = Date.now() - startTime;
            resultsDiv.innerHTML = `❌ Error testing ${endpoint}\n📊 Error: ${error.message}\n⏱️ Failed after: ${responseTime}ms`;
            showNotification(`❌ Failed to reach ${endpoint}`, 'error');
        });
}

// Show notification system
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    
    const colors = {
        success: '#28a745',
        error: '#dc3545', 
        warning: '#ffc107',
        info: '#007bff'
    };
    
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: ${colors[type] || colors.info};
        color: white;
        padding: 15px 20px;
        border-radius: 8px;
        box-shadow: 0 4px 12px rgba(0,0,0,0.2);
        z-index: 10000;
        max-width: 350px;
        font-weight: 600;
        animation: slideInRight 0.3s ease;
        cursor: pointer;
    `;
    
    notification.innerHTML = message;
    document.body.appendChild(notification);
    
    // Auto remove after 4 seconds
    setTimeout(() => {
        notification.style.animation = 'slideOutRight 0.3s ease';
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 300);
    }, 4000);
    
    // Remove on click
    notification.addEventListener('click', function() {
        this.style.animation = 'slideOutRight 0.3s ease';
        setTimeout(() => {
            if (this.parentNode) {
                this.parentNode.removeChild(this);
            }
        }, 300);
    });
}

// Start status updates
function startStatusUpdates() {
    // Update server status every 30 seconds
    setInterval(() => {
        updateServerStatus();
    }, 30000);
    
    // Performance monitoring
    monitorPerformance();
}

// Update server status
function updateServerStatus() {
    fetch('/')
        .then(response => {
            if (response.ok) {
                console.log('🟢 Server status: Online');
                updateStatusDot('online');
            } else {
                console.log('🟡 Server status: Issues detected');
                updateStatusDot('warning');
            }
        })
        .catch(error => {
            console.log('🔴 Server status: Offline');
            updateStatusDot('offline');
        });
}

// Update status indicator
function updateStatusDot(status) {
    const statusElement = document.querySelector('.server-status');
    if (!statusElement) return;
    
    const colors = {
        online: '#28a745',
        warning: '#ffc107',
        offline: '#dc3545'
    };
    
    const messages = {
        online: 'Server Online',
        warning: 'Server Warning', 
        offline: 'Server Offline'
    };
    
    statusElement.style.backgroundColor = colors[status];
    statusElement.querySelector('span:last-child').textContent = messages[status];
}

// Performance monitoring
function monitorPerformance() {
    if (!('performance' in window)) return;
    
    window.addEventListener('load', function() {
        setTimeout(() => {
            const perfData = performance.getEntriesByType('navigation')[0];
            
            const metrics = {
                'DNS Lookup': perfData.domainLookupEnd - perfData.domainLookupStart,
                'TCP Connect': perfData.connectEnd - perfData.connectStart,
                'Request Time': perfData.responseStart - perfData.requestStart,
                'Response Time': perfData.responseEnd - perfData.responseStart,
                'DOM Processing': perfData.domContentLoadedEventEnd - perfData.responseEnd,
                'Total Load': perfData.loadEventEnd - perfData.navigationStart
            };
            
            console.group('⚡ Performance Metrics');
            Object.entries(metrics).forEach(([key, value]) => {
                console.log(`${key}: ${value.toFixed(2)}ms`);
            });
            console.groupEnd();
            
            // Show performance notification if load time is good
            if (metrics['Total Load'] < 1000) {
                showNotification(`⚡ Fast load time: ${metrics['Total Load'].toFixed(0)}ms`, 'success');
            }
            
        }, 100);
    });
}

// Add CSS animations
const style = document.createElement('style');
style.textContent = `
    @keyframes slideInRight {
        from { transform: translateX(100%); opacity: 0; }
        to { transform: translateX(0); opacity: 1; }
    }
    
    @keyframes slideOutRight {
        from { transform: translateX(0); opacity: 1; }
        to { transform: translateX(100%); opacity: 0; }
    }
    
    .notification {
        transition: all 0.3s ease;
    }
    
    .feature-card {
        opacity: 0;
        transform: translateY(20px);
        transition: all 0.3s ease;
    }
`;
document.head.appendChild(style);

// Utility functions for external use
window.serverUtils = {
    testEndpoint: testEndpoint,
    showNotification: showNotification,
    getServerInfo: () => ({ ...serverInfo, uptime: Date.now() - serverInfo.startTime }),
    
    // Test all endpoints
    testAllEndpoints: function() {
        const endpoints = ['/', '/css/style.css', '/js/script.js', '/nonexistent'];
        endpoints.forEach((endpoint, index) => {
            setTimeout(() => testEndpoint(endpoint), index * 1000);
        });
    },
    
    // Show server statistics
    showStats: function() {
        const info = this.getServerInfo();
        const stats = [
            `🚀 Server Uptime: ${formatUptime(info.uptime)}`,
            `📊 Requests Made: ${info.requestCount}`,
            `🌐 Current URL: ${window.location.href}`,
            `💻 User Agent: ${navigator.userAgent.split(' ')[0]}`,
            `📱 Platform: ${navigator.platform}`
        ].join('\n');
        
        showNotification(stats, 'info');
    }
};

// Welcome message
console.log(`
🚀 Multithreaded Web Server Dashboard
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Static file serving active
📊 Interactive dashboard loaded
🧪 Testing utilities available
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

💡 Try these commands in console:
   serverUtils.testAllEndpoints()
   serverUtils.showStats()
   serverUtils.testEndpoint('/custom-path')
`);

// Show initial success message
setTimeout(() => {
    showNotification('🎉 Web server dashboard loaded successfully!', 'success');
}, 1500);
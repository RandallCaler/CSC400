document.addEventListener('DOMContentLoaded', () => {
    let currentSection = 0; // Track the current section

    // Display the first section by default
    showSection(currentSection);

    window.scrollToCell = function(targetSection) {
        if (targetSection === currentSection) return; // Prevent unnecessary animations

        const currentSectionEl = document.getElementById(`smartCell${currentSection}`);
        const targetSectionEl = document.getElementById(`smartCell${targetSection}`);

        // Determine direction
        const direction = targetSection > currentSection ? 'right' : 'left';

        const targetContainer = document.getElementById(`container`);

        // Add animation classes for both current and target sections
        currentSectionEl.classList.add(`${direction}-scroll-exit`);
        targetSectionEl.classList.add(`${direction}-scroll-enter`, 'active');

        // Wait for animation to finish
        setTimeout(() => {
            // Remove animation classes
            currentSectionEl.classList.remove('active', `${direction}-scroll-exit`);
            targetSectionEl.classList.remove(`${direction}-scroll-enter`);
            currentSection = targetSection; // Update current section
        }, 1000); // Match the animation duration
    };

    function showSection(sectionIndex) {
        const sectionEl = document.getElementById(`smartCell${sectionIndex}`);
        if (sectionEl) {
            sectionEl.classList.add('active');
        }
    }
});

function flashSection(sectionId) {
    var section = document.getElementById(sectionId);
    section.classList.add('flash'); // Add the 'flash' class
    setTimeout(function() {
        section.classList.remove('flash'); // Remove the 'flash' class after 1 second
    }, 2500);
}
